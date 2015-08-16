#ifndef LYTHON_PARSER_MODULE_HEADER
#define LYTHON_PARSER_MODULE_HEADER

#include <boost/noncopyable.hpp>
#include <unordered_map>


#include "../AbstractSyntaxTree/Expression.h"

namespace LIBNAMESPACE
{

// Hold Parsed AST for later
// Modules are unique
class Module : private boost::noncopyable
{
public:
    ef std::string Signature;
    ef AST::Expression Object ;
    ef std::unordered_map<Signature, Object*> ObjectHolder;

    Module(const std::string& name, bool owning=true):
        _name(name), _owner(owning)
    {}

    ~Module()
    {
        if (_owner)
            for(auto i:_scope)
                delete i.second;
    }

    //  1 Success   - Object added
    //  0 Error     - The signature is already used
    inline bool add_definition(const Signature& s, Object* obj)
    {
        return _scope.insert({s, obj}).second;
    }

    // Merge a new
//    Module merge(const std::string& s3,
//                 const std::string& s1,
//                 const Module& m1,
//                 const std::string& s2,
//                 const Module& m2)
//    {
//        Module m3(s3, false);

//        for(auto i: m1.ObjectHolder)
//            m3.add_definition(s1 + i.first, i.second);

//        for(auto i: m2.ObjectHolder)
//            m3.add_definition(s2 + i.first, i.second);

//        return m3;
//    }

    //void is_declared(Object* obj);
    inline bool is_declared(const Signature& s)         {   return _scope.count(s); }
    inline void override(const Signature& s, Object* a) {   _scope[s] = a; }

    inline const Object* operator[](const Signature& s)
    {
        return _scope[s];
    }

    void print(std::ostream& out, AST::Expression* e)
    {
        AST::ExpressionType t = e->etype;

        switch(t)
        {
            case AST::Type_VariableExpression:
                out << "Variable";
                return;

            case AST::Type_ClassExpression:
                out << "Class Declaration";
                return;

            case AST::Type_Function:
                out << "Function Declaration";
                return;

            case AST::Type_BinaryExpression:{
                AST::BinaryExpression* b = (AST::BinaryExpression*) e;

                if (b->lhs->etype == AST::Type_VariableExpression)
                    out << "Variable";

                if (b->rhs->etype == AST::Type_Expression){
                    out << " ";
                    b->rhs->print(out);
                }
                return;
        }

            default:
                out  << t;
                return;
        }
    }

    void print(std::ostream& out)
    {
        out << "Loaded Object in '" << _name << "' module: {\n";

        for(auto i:_scope)
        {
            out << "    " << i.first << " : ";

            print(out, i.second);

            out << "\n";
        }

        out << "}\n";

    }

private:
    ObjectHolder _scope;
    std::string  _name;
    bool _owner;
};

}

#endif
