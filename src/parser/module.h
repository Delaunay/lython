#pragma once

#include <sstream>
#include <unordered_set>

#include "logging/logging.h"
#include "dtypes.h"
#include "ast/expressions.h"


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
    static Expression type_type();
    static Expression float_type();

    void insert_builtin();

    Module(Module const* parent = nullptr, int depth = 0, int offset = 0):
        depth(depth), offset(offset), _parent(parent)
    {
        insert_builtin();
    }

    int size() const {
        return int(offset) + _scope.size();
    }

    Module enter() const {
        auto m = Module(this, this->depth + 1, size());
        return m;
    }

    // make a reference from a name
    Expression reference(String const &view) const;

    int insert(String const& name, Expression const& expr){
        // info("Inserting Expressionession");
        auto idx = _scope.size();
        _name_idx[name] = idx;
        _idx_name.push_back(name);
        _scope.push_back(expr);

        return idx;
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

    int _find_index(String const &view) const {
        // Check in the current scope
        auto iter = _name_idx.find(view);
        int idx = -1;

        if (iter != _name_idx.end()){
            idx = (*iter).second + offset;
        } else if (_parent){
            idx = _parent->_find_index(view);
        }

        return idx;
    }

    std::ostream& print(std::ostream& out, int depth = 0) const;

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
    Array<Expression> _scope;
    Array<String>     _idx_name;
    Dict<String, int> _name_idx;
};

} // namespace lython
