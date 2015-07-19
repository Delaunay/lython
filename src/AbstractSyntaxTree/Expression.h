/* *****************************************************************************
 *
 *  Implementation of methods are separated in two files:
 *          - CodeGen.cpp for methods generating IR
 *          - Expression.cpp for everything else
 *
 * ****************************************************************************/
#ifndef LYTHON_AST_EXPR_HEADER
#define LYTHON_AST_EXPR_HEADER

#include "../Generator/Generator.h"

#include <string>
#include <vector>
#include <iostream>

#if LLVM_CODEGEN
#   define DEFINE_CODE_GEN virtual llvm::Value* code_gen(Generator& g);
#else
#   define DEFINE_CODE_GEN
#endif

using namespace std;

namespace lython
{
namespace AbstractSyntaxTree
{
    enum ExpressionType
    {
        Type_Expression,
        Type_BinaryExpression,
        Type_CallExpression,
        Type_ForExpression,
        Type_Function,
        Type_IfExpression,
        Type_MultilineExpression,
        Type_MutableVariableExpression,
        Type_Prototype,
        Type_TypedExpression,
        Type_UnaryExpression,
        Type_VariableExpression
    };

    // generate correct indent
    string I(unsigned int n);

    class Expression
    {
        public:
            Expression(ExpressionType t = Type_Expression, bool tp = false):
                type(t), complex(tp)
            {}

            virtual ~Expression();

            virtual void print(ostream& str, int i = 0);

            const ExpressionType type;

            // Complex structure cant be inside Operators
            // imagine incrementing a for loop. it makes no sens
            const bool           complex;

//            virtual const string idendity();

        #if LLVM_CODEGEN
            virtual llvm::Value* code_gen(Generator& g) = 0;
        #endif
    };

    template<typename T = double>
    class TypedExpression : public Expression
    {
        public:
            TypedExpression(T v):
                Expression(Type_TypedExpression), value(v)
            {}

            const string idendity() {   return "Typed Expression"; }

            void print(ostream& str, int i = 0)
            {
                str << I(i) << value;
            }

        #if LLVM_CODEGEN
            virtual llvm::Value* code_gen(Generator& g)
            {
                return llvm::ConstantFP::get(llvm::getGlobalContext(),
                                             llvm::APFloat(value));
            }
        #endif

            T value;
    };

    typedef TypedExpression<double> DoubleExpression;
    typedef TypedExpression<float>  FloatExpression;
    typedef TypedExpression<int>    IntExpression;
    typedef TypedExpression<std::string>    StringExpression;

    /// VarExprAST - Expression class for var/in
    class MutableVariableExpression : public Expression
    {
        public:
            MutableVariableExpression(
                    const vector<pair<string, Expression*> > &varnames,
                    Expression *body):
              var_names(varnames), body(body), Expression(Type_MutableVariableExpression)
            {}

            const string idendity();

            DEFINE_CODE_GEN

        vector<pair<string, Expression*> > var_names;
        Expression* body;
    };

    /// VariableExprAST - Expression class for referencing a variable, like "a"
    class VariableExpression: public Expression
    {
        public:
            VariableExpression(const string &name);
            void print(ostream& str, int i = 0);

            const string idendity();

            DEFINE_CODE_GEN

        string name;
    };

    // lhs + rhs ===> 'operator'(lhs, rhs)
    class BinaryExpression: public Expression
    {
        public:

            BinaryExpression(string op, Expression *lhs, Expression *rhs);

            const string idendity();

            // 'operator'(lhs, rhs)
            void print(ostream& str, int i = 0);

            DEFINE_CODE_GEN

            string        op;
            Expression *lhs;
            Expression *rhs;
    };

    // represent a calling inside an expression
    class CallExpression: public Expression
    {
        public:
            typedef vector<Expression*> Arguments;

            CallExpression(const std::string &callee, Arguments& args);

            const string idendity();

            void print(ostream& str, int i = 0);

            DEFINE_CODE_GEN

            string callee;
            Arguments args;
    };

    // def function(args1, args2, ...) => fn name(args1, args2, ...)
    class Prototype
    {
        public:
            typedef vector<string> Arguments;

            Prototype(const string &name, const Arguments& args,
                      bool isoperator = false, unsigned prec = 0);

            const string idendity();

            void print(ostream& str, int i = 0);

            DEFINE_CODE_GEN

        #if LLVM_CODEGEN
            void create_argument_allocas(Generator &g, llvm::Function* f);
        #endif

            bool        is_unary() const;
            bool        is_binary() const;
            const bool& is_operator() const;

            const unsigned& precedence() const;

            string operator_name() const;
            string name;
            Arguments args;

            // user defined operators

            unsigned _precedence;
            bool _is_operator;

    };

    // prototype  => fn name(args1, args2, args3, ...)
    // expression =>
    class Function
    {
        public:
            Function(Prototype *proto, Expression *body);

            void print(ostream& str, int i = 0);

            const string idendity();

            DEFINE_CODE_GEN

            Prototype* prototype;
            Expression* body;
    };

    class IfExpression : public Expression
    {
        public:
            IfExpression(Expression* cond, Expression* then, Expression* lse);

            void print(ostream& str, int i = 0);

            const string idendity();

            DEFINE_CODE_GEN

            Expression* cond;
            Expression* then;
            Expression* els;
    };


    class ForExpression : public Expression
    {
        public:
            // C for
            ForExpression(const std::string &var, Expression* s, Expression* e,
                                                  Expression* st, Expression* bd);

            // Python for
            ForExpression(const std::string &var, Expression*s, Expression* bd);

            DEFINE_CODE_GEN
            const string idendity();

            void print(ostream& str, int i = 0);

            string      var;
            Expression* start;
            Expression* end;
            Expression* step;
            Expression* body;
    };

    class UnaryExpression : public Expression
    {
        public:

            UnaryExpression(string opcode, Expression *operand);

            void print(ostream& str, int i = 0);

            const string idendity();

            DEFINE_CODE_GEN

        string opcode;
        Expression* operand;
    };

    class MultilineExpression : public Expression
    {
        public:

            MultilineExpression():
                Expression(Type_MultilineExpression)
            {}

            ~MultilineExpression()
            {}

            virtual void print(ostream& str, int i = 0)
            {
                for(int k = 0; k < expressions.size(); k++)
                {
                    if (expressions[k]->type == Type_MultilineExpression)
                        expressions[k]->print(str, i + 1);
                    else
                        expressions[k]->print(str, i);

                    str << "\n";
                }
            }

            void add(Expression* exp)
            {
                if (exp != 0)
                    expressions.push_back(exp);
            }

            DEFINE_CODE_GEN

            vector<Expression*> expressions;
    };
}

namespace AST = AbstractSyntaxTree;
}

/*
template<>
class TypedExpression<int> : public Expression
{
    public:
        TypedExpression(int v):
            value(v)
        {}

        void print(ostream& str, int i = 0)
        {
            str << value;
        }

    #if LLVM_CODEGEN
        virtual llvm::Value* code_gen(Generator& g)
        {
            return llvm::ConstantFP::get(llvm::getGlobalContext(),
                                         llvm::APInt());
        }
    #endif

        int value;
};*/
#endif
