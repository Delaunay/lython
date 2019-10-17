#pragma once

#include <sstream>
#include <unordered_set>

#include "../logging/logging.h"
#include "../Types.h"
#include "../utilities/trie.h"
#include "ast/expressions.h"
#include "fmt.h"


// This is made to indicate if the mapped value was set or not
struct Index{
    int val;

    Index(int value = -1):
        val(value)
    {}

    Index(size_t value):
        val(int(value))
    {}

    operator size_t(){
        assert(val >= 0);
        return size_t(val);
    }

    operator int(){
        return val;
    }

    operator bool(){ return val >= 0; }

    template<typename T> bool operator < (T i) { return val < int(i);}
    template<typename T> bool operator > (T i) { return val > int(i);}
    template<typename T> bool operator== (T i) { return val == int(i);}
    template<typename T> bool operator!= (T i) { return val != int(i);}
    template<typename T> Index operator+ (T i) {
        return Index(val + int(i));
    }
    template<typename T>
    Index& operator+= (T i) {
        val += i;
        return *this;
    }
    template<typename T>
    Index& operator-= (T i) {
        val -= i;
        return *this;
    }
    Index& operator++ () {
        val += 1;
        return *this;
    }
    Index& operator-- () {
        val -= 1;
        return *this;
    }
};


namespace lython {
//  Module
// -------------------------------------

// Holds Currently defined entities

// I want AST::Expr to be hashable
struct expr_hash {
    std::size_t operator()(const ST::Expr &v) const noexcept;
    std::hash<std::string> _h;
};

struct expr_equal {
    bool operator()(const ST::Expr &a, const ST::Expr &b) const noexcept;
};

// using BaseScope = std::unordered_set<ST::Expr, expr_hash, expr_equal>;

// ---
/*
 * Basic Module, used by the parser to keep track of every definition
 * and make sure everything is reachable from a given scope.
 * It only saves the Name and the Expression / Type corresponding
 *
 * Evaluation use a different kind of scope.
 */
class Module {
  public:
    static ST::Expr type_type() {
        static ST::Expr type = std::make_shared<AST::Type>("Type");
        return type;
    }

    static ST::Expr float_type() {
        static ST::Expr type =
            std::make_shared<AST::Builtin>("Float", type_type());
        return type;
    }

    Module(Module const* parent = nullptr, int depth = 0, int offset = 0):
        depth(depth), offset(offset), _parent(parent)
    {
        if (!parent){
            insert("Type", type_type());
            insert("Float", float_type());
            for (auto c : {"+", "-", "*", "/", ".*", "./", "%", "^"}) {
                _operators.insert(c);
            }

            auto min_type = new AST::Arrow();
            min_type->params.push_back(
                AST::Parameter(make_name("a"), float_type()));
            min_type->params.push_back(
                AST::Parameter(make_name("b"), float_type()));

            auto sin_type = new AST::Arrow();
            sin_type->params.push_back(AST::Parameter(make_name("a"), float_type()));

            auto double_double = ST::Expr(min_type);
            auto min_fun = new AST::Builtin("min", double_double);
            auto max_fun = new AST::Builtin("max", double_double);
            auto sin_fun = new AST::Builtin("sin", ST::Expr(sin_type));

            insert("min", ST::Expr(min_fun));
            insert("sin", ST::Expr(sin_fun));
            insert("max", ST::Expr(max_fun));
        }
    }

    Index size() const {
        return Index(offset) + _scope.size();
    }

    Module enter() const {
        auto m = Module(this, this->depth + 1, size());
        m._operators = this->_operators;
        m._precedence_table = this->_precedence_table;
        return m;
    }

    Dict<String, std::tuple<int, bool>> &precedence_table() {
        return _precedence_table;
    }

    Trie<128> const *operator_trie() const { return &_operators.trie(); }

    static Dict<String, std::tuple<int, bool>> default_precedence() {
        static Dict<String, std::tuple<int, bool>> val = {
            {"+", {2, true}}, // Predecence, Left Associative
            {"-", {2, true}}, {"%", {1, true}},  {"*", {3, true}},
            {"/", {3, true}}, {".*", {2, true}}, {"./", {2, true}},
            {"^", {4, false}}};
        return val;
    }

    int arg_count(std::string_view view) const {
        thread_local String _tmp(view.begin(), view.end());
        ST::Expr fun_expr = this->find(_tmp);

        if (fun_expr != nullptr &&
            fun_expr->kind() == AST::Expression::KindFunction) {
            auto fun = static_cast<AST::Function *>(fun_expr.get());
            return int(fun->args().size());
        }

        return -1;
    }

    Index insert(String const& name, ST::Expr const& expr){
        // info("Inserting ST::Expression");
        auto idx = _scope.size();
        _name_idx[name] = idx;
        _idx_name.push_back(name);
        _scope.push_back(expr);
        return idx;
    }

    ST::Expr operator[](int idx) const {
        return get_item(idx);
    }
    ST::Expr get_item(int idx) const {
        if (idx >= offset){
            return _scope[size_t(idx - offset)];
        }
        if (_parent){
            return _parent->get_item(idx);
        }
        return nullptr;
    }

    Index find_index(String const &view) const {
        // Check in the current scope
        auto iter = _name_idx.find(view);

        if (iter != _name_idx.end()){
            Index idx = (*iter).second;
            return idx + offset;
        } else if (_parent){
            return _parent->find_index(view);
        }

        return Index(-1);
    }

    ST::Expr find(String const &view) const {
        // Check in the current scope
        auto iter = _name_idx.find(view);

        if (iter != _name_idx.end()){
            Index idx = (*iter).second;
            return _scope[idx];
        } else if (_parent){
            return _parent->find(view);
        }

        return nullptr;
    }

    template<typename T>
    T replace(T const& t, char a, T const& b) const{
        Index n = t.size();
        auto iter = t.rbegin();
        while (*iter == '\n'){
            n -= 1;
            iter -= 1;
        }

        int count = 0;
        for (Index i = 0; i < n; ++i){
            if (t[i] == a){
                count += 1;
            }
        }

        auto str = T(n + b.size() * size_t(count), ' ');
        Index k = 0;
        for(Index i = 0; i < n; ++i){
            if (t[i] != a){
                str[k] = t[i];
                k += 1;
            } else {
                for(Index j = 0; j < b.size(); ++j){
                    str[k] = b[j];
                    k += 1;
                }
            }
        }

        return str;
    }

    std::ostream& print(std::ostream& out, int depth = 0) const{
        auto line = String(80, '-');

        if (depth == 0){
            out << "Module dump print:\n";
        }

        if (!_parent){
            out << line << "\n";
            out << align_right("id", 4) << "   ";
            out << align_right("name", 30) << "   type\n";
            out << line << "\n";
        }
        else{
            _parent->print(out, depth + 1);
            out << String(40, '-') << "\n";
        }
        for(Index i = 0; i < _scope.size(); ++i){
            auto name = _idx_name[i];
            auto expr = _scope[i];

            out << to_string(int(i), 4) << "   ";
            out << align_right(name, 30) << "   ";

            std::stringstream ss;
            expr->print(ss) << "\n";

            auto str = ss.str();

            out << replace(str, '\n', "\n" + std::string(40, ' ')) << '\n';
        }

        if (depth == 0){
            out << line << "\n";
        }
        return out;
    }

    class ModuleIterator{
    public:
        ModuleIterator(Module const& iter, std::size_t index = 0):
            iter(iter), index(int(index))
        {}

        bool operator==(ModuleIterator& iter){
            return this->index == iter.index;
        }

        bool operator!=(ModuleIterator& iter){
            return this->index != iter.index;
        }

        operator bool() const {
            return std::size_t(this->index) == this->iter._scope.size();
        }

        std::pair<String, ST::Expr> operator*(){
            return {
                iter._idx_name[std::size_t(index)],
                iter._scope[std::size_t(index)]
            };
        }

        std::pair<String, ST::Expr> operator*() const{
            return {
                iter._idx_name[std::size_t(index)],
                iter._scope[std::size_t(index)]
            };
        }

        ModuleIterator& operator++(){
            index += 1;
            return *this;
        }

        ModuleIterator& operator--(){
            index = std::max(index - 1, 0);
            return *this;
        }

    private:
        Module const& iter;
        int index;
    };

    ModuleIterator       begin()        { return ModuleIterator(*this); }
    ModuleIterator const begin() const  { return ModuleIterator(*this); }
    ModuleIterator       end()          { return ModuleIterator(*this, _scope.size()); }
    ModuleIterator const end() const    { return ModuleIterator(*this, _scope.size()); }

  private:
    //
    int const depth;
    int const offset = 0;
    Module const* _parent = nullptr;

    // stored in an array so we can do lookup by index
    Array<ST::Expr> _scope;
    Array<String>   _idx_name;
    Dict<String, Index> _name_idx;

    // This is more for parsing
    // they are copied everytime a new scope is created
    // will see if it will be an issue
    CoWTrie<128> _operators;
    Dict<String, std::tuple<int, bool>> _precedence_table =
        default_precedence();
};

} // namespace lython
