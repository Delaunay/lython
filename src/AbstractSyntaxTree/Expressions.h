#pragma once
/*
 *  What is a Program ?
 *
 *      You've got known and unknown. Program use known data to compute
 *      unknown using specified procedures.
 *
 *  For example:
 *      -> runtime known -> Value changes
 *      -> compile known ->
 */

#include <memory>
#include "Names.h"

// declare common function
// I don't have to add them to the class declaration one by one
// not best design but since I am experiencing this might change
// quite a lot
#define LYTHON_COMMFUNC(type, body)\
    virtual type partial_eval() body\
    virtual type derivate() body \
    virtual std::ostream& print(std::ostream&) body

#define LYTHON_COMMFUNCCHILD LYTHON_COMMFUNC(ST::Expr, {})

namespace lython{

// Private Object
namespace AbstractSyntaxTree{
    class Expression
    {
        public:
            LYTHON_COMMFUNC(std::shared_ptr<Expression>, {})

        private:
    };
}
namespace AST = AbstractSyntaxTree;

// Public Object
namespace SyntaxTree{
    typedef std::shared_ptr<AST::Expression> Expr;
}
namespace ST = SyntaxTree;

namespace AbstractSyntaxTree{
    // We declare the leafs of our program
    // -----------------------------------
    
    // A Placeholder is a special construct that represent a unknown value
    // that is unknown at compile time but will be known at runtime
    class Placeholder : public Expression
    {
        public:
            Placeholder(const std::string& name, const std::string& type):
                _name(make_name(name)), _type(make_type(type))
            {}

            Placeholder(Name name, Type type):
                _name(name), _type(type)
            {}

            Name& name() {  return _name;   }
            Type& type() {  return _type;   }

            LYTHON_COMMFUNCCHILD
        
        private:
            Name _name;
            Type _type; // Only used for compile type
                        // type info are discarded later
    };
    
    // A Constant is a value known at compile time
    template<typename T>
    class Constant : public Expression
    {
        public:
            Constant(T value, const std::string& type):
                _value(value), _type(make_type(type))
            {}

            Constant(T value, Type type):
                _value(value), _type(type)
            {}

            T value()    {  return _value;  }
            Type& type() {  return _type;   }

            LYTHON_COMMFUNCCHILD
            
        private:
            T    _value;
            Type _type;
    };

    // We declare Basic nodes of our program
    // -------------------------------------

    // A binary operator is a function with two parameters
    // Some language specify binary operator as function
    // we want our language to be readable
    class BinaryOperator : public Expression
    {
        public:
            BinaryOperator(ST::Expr rhs, ST::Expr lhs, Operator op):
                _rhs(rhs), _lhs(lhs), _op(op)
            {}

            LYTHON_COMMFUNCCHILD

        private:
            ST::Expr _rhs;
            ST::Expr _lhs;
            Operator _op;
    };

    class UnaryOperator: public Expression
    {
        public:
            UnaryOperator(ST::Expr expr, Operator op):
                _expr(expr), _op(op)
            {}

            LYTHON_COMMFUNCCHILD

        private:
            ST::Expr _expr;
            Operator _op;
    };
}
}

/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
