#pragma once

#include <sstream>
#include <unordered_set>

#include "logging/logging.h"
#include "dtypes.h"
#include "ast/expressions.h"
#include "ast/nodes.h"

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
        assert(val >= 0, "");
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
    template<typename T> Index operator+ (T i) const {
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

template <typename T>
bool operator < (T i, Index a){ return a > i; }


namespace lython {
//  Module
// -------------------------------------

// Holds Currently defined entities

// I want AExpression to be hashable
struct expr_hash {
    std::size_t operator()(const Expression &v) const noexcept;
    std::hash<std::string> _h;
};

struct expr_equal {
    bool operator()(const Expression &a, const Expression &b) const noexcept;
};

// using BaseScope = std::unordered_set<Expression, expr_hash, expr_equal>;


struct AccessTracker{
    Array<Tuple<String, int>> access;

    void add_access(String const& v, int i){
        for(auto j: access){
            if (std::get<1>(j) == i)
                return;
        }
        access.emplace_back(v, i);
    }
};

// ---
/*
 * Basic Module, used by the parser to keep track of every definition
 * and make sure everything is reachable from a given scope.
 * It only saves the Name and the Expression / Type corresponding
 *
 * Evaluation use a different kind of scope.
 *
 * AccessTracker is used to keep track of all the used Expression for functions
 */
class Module {
  public:
    static Expression type_type() {
        static auto type = Expression::make<AST::Type>("Type");
        return type;
    }

    static Expression float_type() {
        static auto type =
            Expression::make<AST::Builtin>("Float", type_type(), 1);
        return type;
    }

    Module(Module const* parent = nullptr, int depth = 0, int offset = 0, AccessTracker* tracker = nullptr):
        depth(depth), offset(offset), _parent(parent), _tracker(tracker)
    {
        if (!parent){
            insert("Type", type_type());
            insert("Float", float_type());

            auto f_f_f = Expression::make<AST::Arrow>();
            auto binary_type = f_f_f.ref<AST::Arrow>();
            binary_type->params.push_back(AST::Parameter("a", float_type()));
            binary_type->params.push_back(AST::Parameter("b", float_type()));

            auto f_f = Expression::make<AST::Arrow>();
            auto unary_type = f_f_f.ref<AST::Arrow>();
            unary_type->params.push_back(AST::Parameter("a", float_type()));

            auto min_fun = Expression::make<AST::Builtin>("min", f_f_f, 2);
            auto max_fun = Expression::make<AST::Builtin>("max", f_f_f, 2);
            auto sin_fun = Expression::make<AST::Builtin>("sin", f_f, 1);
            auto pi = Expression::make<AST::Value>(3.14, float_type());

            insert("min", Expression(min_fun));
            insert("sin", Expression(sin_fun));
            insert("max", Expression(max_fun));
            insert("pi", Expression(pi));
        }
    }

    Index size() const {
        return Index(offset) + _scope.size();
    }

    Module enter(AccessTracker* tracker = nullptr) const {
        auto m = Module(this, this->depth + 1, size(), tracker);
        return m;
    }

    int arg_count(std::string_view view) const {
        thread_local String _tmp(view.begin(), view.end());
        Expression fun = this->find(_tmp);

        if (fun && fun.kind() == AST::NodeKind::KFunction) {
            return int(fun.ref<AST::Function>()->args.size());
        }

        return -1;
    }

    Index insert(String const& name, Expression const& expr, bool block_idx=false){
        // info("Inserting Expressionession");
        auto idx = _scope.size();
        _name_idx[name] = idx;
        _idx_name.push_back(name);
        _scope.push_back(expr);

        if (_tracker){
            _tracker->add_access(name, int(idx) + offset);
        }

        if (!block_idx)
            return idx;

        return int(idx) - offset;
    }

    Expression operator[](int idx) const {
        return get_item(idx);
    }
    Expression get_item(int idx) const {
        if (idx >= offset){
            return _scope[size_t(idx - offset)];
        }
        if (_parent){
            return _parent->get_item(idx);
        }
        return Expression();
    }

    String get_name(int idx) const {
        if (idx >= offset){
            return _idx_name[idx];
        }
        if (_parent){
            return _parent->get_name(idx);
        }
        return "nullptr";
    }

    Index find_index(String const &view, bool block_loc=false) const {
        // Check in the current scope
        auto iter = _name_idx.find(view);
        Index idx = -1;

        if (iter != _name_idx.end()){
            idx = (*iter).second + offset;
        } else if (_parent){
            idx = _parent->find_index(view);
        }

        if (_tracker){
            _tracker->add_access(view, idx);
        }
        if (!block_loc)
            return idx;

        return int(idx) - offset;
    }

    Expression find(String const &view) const {
        Expression expr;
        Index idx = 0;

        std::tie(expr, idx) = _find(view);

        // TODO: might be interesting to create a MissingExpr in the AST
        // itself for rendering in TIDE
        // when expr == nullptr
        if (expr && expr.kind() == AST::NodeKind::KReference)
            return expr;

        return Expression::make<AST::Ref>(
            String(view),
            idx,
            size(),
            Expression());
    }

    int get_nargs(Expression& fun) const {
        if (!fun)
            return -1;

        if (fun.kind() == AST::NodeKind::KFunction){
            return int(fun.ref<AST::Function>()->args.size());
        }

        if (fun.kind() == AST::NodeKind::KBuiltin){
            return int(fun.ref<AST::Builtin>()->argument_size);
        }

        if (fun.kind() == AST::NodeKind::KStruct){
            return int(fun.ref<AST::Struct>()->attributes.size());
        }

        return -1;
    }


    Expression make_ref(String const &view, int idx, Expression type=Expression()) const {
        return Expression::make<AST::Ref>(
            String(view),
            idx,
            size(),
            type);
    }

    Tuple<Expression, int> find_function(String const &view) const {
        Expression expr;
        Index idx = 0;
        int nargs = -1;

        std::tie(expr, idx) = _find(view);

        nargs = get_nargs(expr);

        return std::make_tuple(make_ref(view, idx), nargs);
    }

    Tuple<Expression, Index> _find(String const &view) const {
        // Check in the current scope
        auto iter = _name_idx.find(view);

        if (iter != _name_idx.end()){
            debug("found {}", view.c_str());
            Index idx = (*iter).second;

            return std::make_tuple(_scope[idx], idx);

        } else if (_parent){
            return _parent->_find(view);
        }

        return std::make_tuple(Expression(), -1);
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
            expr.print(ss) << "\n";

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

        std::pair<String, Expression> operator*(){
            return {
                iter._idx_name[std::size_t(index)],
                iter._scope[std::size_t(index)]
            };
        }

        std::pair<String, Expression> operator*() const{
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
    AccessTracker*      _tracker = nullptr;
    Array<Expression>   _scope;
    Array<String>       _idx_name;
    Dict<String, Index> _name_idx;
};

} // namespace lython
