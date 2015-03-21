#ifndef LYTHON_AST_EXPR_HEADER
#define LYTHON_AST_EXPR_HEADER

#include "../Generator/Generator.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

namespace lython
{
namespace AbstractSyntaxTree
{
    class Expression
    {
        public:
            virtual ~Expression();
            virtual void print(ostream& str, int i = 0);

        #if LLVM_CODEGEN
            virtual llvm::Value* code_gen(Generator& g) = 0;
        #endif
    };

    template<typename T>
    class TypedExpression : public Expression
    {
        public:
            TypedExpression(T v):
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
                                             llvm::APFloat(value));
            }
        #endif

            T value;
    };

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

    typedef TypedExpression<double> DoubleExpression;
    typedef TypedExpression<float>  FloatExpression;
    typedef TypedExpression<int>    IntExpression;

    class VariableExpression: public Expression
    {
        public:
            VariableExpression(const string &name);
            void print(ostream& str, int i = 0);

        #if LLVM_CODEGEN
            virtual llvm::Value* code_gen(Generator& g);
        #endif

            string name;
    };

    // lhs + rhs ===> 'operator'(lhs, rhs)
    class BinaryExpression: public Expression
    {
        public:

            BinaryExpression(char op, Expression *lhs, Expression *rhs);

            // 'operator'(lhs, rhs)
            void print(ostream& str, int i = 0);

        #if LLVM_CODEGEN
            virtual llvm::Value* code_gen(Generator& g);
        #endif

            char        op;
            Expression *lhs;
            Expression *rhs;
    };

    // represent a calling inside an expression
    //
    class CallExpression: public Expression
    {
        public:
            typedef vector<Expression*> Arguments;

            CallExpression(const std::string &callee, Arguments& args);

            void print(ostream& str, int i = 0);

        #if LLVM_CODEGEN
            virtual llvm::Value* code_gen(Generator& g);
        #endif

            string callee;
            Arguments args;
    };

    // def function(args1, args2, ...) => fn name(args1, args2, ...)
    class Prototype
    {
        public:
            typedef vector<string> Arguments;

            Prototype(const string &name, const Arguments& args);

            void print(ostream& str, int i = 0);

        #if LLVM_CODEGEN
            virtual llvm::Function* code_gen(Generator& g);
        #endif

            string name;
            Arguments args;
    };

    // prototype  => fn name(args1, args2, args3, ...)
    // expression =>
    class Function
    {
        public:
            Function(Prototype *proto, Expression *body);

            void print(ostream& str, int i = 0);

        #if LLVM_CODEGEN
            virtual llvm::Function* code_gen(Generator& g);
        #endif

            Prototype* prototype;
            Expression* body;
    };

    class IfExpression : public Expression
    {
        public:
            IfExpression(Expression* cond, Expression* then, Expression* lse);

            void print(ostream& str, int i = 0);

        #if LLVM_CODEGEN
            virtual llvm::Value* code_gen(Generator& g);
        #endif

            Expression* cond;
            Expression* then;
            Expression* els;
    };


    class ForExpression : public Expression
    {
        public:
            ForExpression(const std::string &var, Expression* s, Expression* e,
                                                  Expression* st, Expression* bd);
        #if LLVM_CODEGEN
            virtual llvm::Value* code_gen(Generator& g);
        #endif

            void print(ostream& str, int i = 0)
            {
                str << "for(" << var << ": ";
                    start->print(str);
                str << " ; ";
                    end->print(str);
                str << " ; ";
                    step->print(str);
                str << ")\n\t";

                body->print(str);
            }

            string      var;
            Expression* start;
            Expression* end;
            Expression* step;
            Expression* body;
    };
}

namespace AST = AbstractSyntaxTree;
}

#endif
