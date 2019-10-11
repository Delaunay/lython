#pragma once

#include <unordered_set>

#include "../Types.h"
#include "../Utilities/trie.h"
#include "AbstractSyntaxTree/Expressions.h"

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

    Module(Module const* parent = nullptr, int depth = 0):
        depth(depth), _parent(parent)
    {
        insert("Type", type_type());
        insert("Float", float_type());
        for (auto c : {"+", "-", "*", "/", ".*", "./", "%", "^"}) {
            _operators.insert(c);
        }

        auto min_type = new AST::Arrow();
        min_type->params.push_back(
            AST::Parameter(make_name("a"), float_type()));
        min_type->params.push_back(
            AST::Parameter(make_name("a"), float_type()));
        auto min_fun = new AST::Builtin("min", ST::Expr(min_type));

        auto max_fun = new AST::Function("max", true);
        max_fun->args().push_back(AST::Parameter(make_name("a"), float_type()));
        max_fun->args().push_back(AST::Parameter(make_name("b"), float_type()));

        auto sin_fun = new AST::Function("sin", true);
        sin_fun->args().push_back(AST::Parameter(make_name("x"), float_type()));

        insert("min", ST::Expr(min_fun));
        insert("sin", ST::Expr(sin_fun));
        insert("max", ST::Expr(max_fun));
    }

    // Copy a module as a submodule of the copied module
    Module(Module const& cpy):
        depth(cpy.depth + 1), _parent(&cpy), _operators(cpy._operators),
        _precedence_table(cpy._precedence_table)
    {}

    Module enter(){
        return Module(*this);
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

    std::size_t insert(String const& name, AST::Expression* expr){
        auto idx = _scope.size();
        _name_idx[name] = idx;
        _idx_name.push_back(name);
        _scope.emplace_back(expr);
        return idx;
    }

    std::size_t insert(String const& name, ST::Expr const& expr){
        auto idx = _scope.size();
        _name_idx[name] = idx;
        _idx_name.push_back(name);
        _scope.push_back(expr);
        return idx;
    }

    ST::Expr insert(AST::Struct *struct_expr) {
        String& name = struct_expr->name();
        auto i = insert(name, struct_expr);
        return _scope[i];
    }

    ST::Expr insert(AST::Function *fun_expr) {
        String& name = fun_expr->name();
        auto i = insert(name, fun_expr);
        return _scope[i];
    }

    ST::Expr find(String const &view) const {
        auto idx = _name_idx.at(view);
        return _scope[idx];
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
    Module const* _parent = nullptr;

    // stored in an array so we can do lookup by index
    Array<ST::Expr> _scope;
    Array<String>   _idx_name;
    Dict<String, std::size_t> _name_idx;

    // This is more for parsing
    // they are copied everytime a new scope is created
    // will see if it will be an issue
    CoWTrie<128> _operators;
    Dict<String, std::tuple<int, bool>> _precedence_table =
        default_precedence();
};

using Name2Idx = Dict<String, std::size_t>;
DEFINE_METADATA(Name2Idx::value_type, Pair[String size_t])

using PrededenceTable = Dict<String, std::tuple<int, bool>>;
DEFINE_METADATA(PrededenceTable::value_type, Pair[String Tuple[int bool]])

DEFINE_METADATA(Array<ST::Expr>::value_type, ST::Expr)


} // namespace lython
