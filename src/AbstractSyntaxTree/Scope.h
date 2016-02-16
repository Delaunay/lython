#ifndef LYTHON_PARSER_Scope_HEADER
#define LYTHON_PARSER_Scope_HEADER

#include "../config.h"

#include <unordered_map>
#include <unordered_set>

//#include "Expression.h"

namespace LIBNAMESPACE
{

namespace AbstractSyntaxTree{
    class ScopedExpression;
    class Expression;
}

namespace AST = AbstractSyntaxTree;

// Hold Parsed AST for later
// Scopes are unique

class Scope
{
public:
    typedef std::string Signature;
    typedef AST::Expression Object ;
    typedef std::unordered_map<Signature, Object*> ObjectHolder;

    Scope(const Scope&) = delete;
    Scope& operator= (const Scope&) = delete;

    Scope(const std::string& name, bool owning=true):
        _name(name), _owner(owning), _parent(nullptr)
    {}

    ~Scope();

    //  1 Success   - Object added
    //  0 Error     - The signature is already used
    inline bool add_definition(const Signature& s, Object* obj)
    {
        if ((_parent != nullptr) && (_parent->is_declared(s)))
            return false;

        return _Scope.insert({s, obj}).second;
    }

//    Merge a new
//    Scope merge(const std::string& s3,
//                 const std::string& s1,
//                 const Scope& m1,
//                 const std::string& s2,
//                 const Scope& m2)
//    {
//        Scope m3(s3, false);

//        for(auto i: m1.ObjectHolder)
//            m3.add_definition(s1 + i.first, i.second);

//        for(auto i: m2.ObjectHolder)
//            m3.add_definition(s2 + i.first, i.second);

//        return m3;
//    }

    //void is_declared(Object* obj);
    inline bool is_declared(const Signature& s)
    {
        if (_parent != nullptr)
            return _Scope.count(s) && _parent->is_declared(s);

        return _Scope.count(s);
    }

    inline void override(const Signature& s, Object* a) {   _Scope[s] = a; }

    inline const Object* operator[](const Signature& s)
    {
        if (_Scope.count(s) > 0)
            return _Scope[s];

        if (_parent != nullptr)
            return (*_parent)[s];

        return nullptr;
    }

    void print(std::ostream& out, AbstractSyntaxTree::Expression *e);
    void print(std::ostream& out);

private:
    Scope*       _parent;
    ObjectHolder _Scope;
    std::string  _name;
    bool _owner;
};
}

#endif
