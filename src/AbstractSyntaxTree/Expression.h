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
#include "../ptr.h"

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#if LLVM_CODEGEN
#   define DEFINE_CODE_GEN virtual llvm::Value* code_gen(Generator& g);
#else
#   define DEFINE_CODE_GEN
#endif

/*
 *      Local Type Inference
 *
 *  We can devide Expression in two classes 'TypeCreator' which have a predetermined type
 *  that cannot change and 'TypeInfered' classes that need to determine their return type
 *  based on
 *
 *  * Type Creator:
 *      - TypedExpression (int, double, string, ...)
 *      - ClassExpression (user defined classes)
 *      - Function        (callbacks variable)
 *      - CallExpression
 *
 *  * Type Infered:
 *      - BinaryExpression
 *      - UnaryExpression
 *      - Function
 *      - VariableExpression
 *
 * Type is infered using TypeCreator's type
 *
 */

using namespace std;

namespace LIBNAMESPACE
{
namespace AbstractSyntaxTree
{
    typedef string Signature;

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
        Type_VariableExpression,
        Type_ClassExpression,

        Type_Int,
        Type_Float,
        Type_String
    };

    enum ValueType
    {
        Double,
        String
    };

    // a lot of Expression dont have types
    // we are going to save some memory by defining a pointer to None
    Pointer<const string>& none_type();
    Pointer<const string>& void_type();     // functions return void by default
    Pointer<const string>& double_type();   // to simplify: everything else is double by default

    // generate correct indent
    string I(unsigned int n);

    class Expression
    {
        public:
            Expression(ExpressionType t = Type_Expression, bool tp = false):
                etype(t), type(none_type()), complex(tp)
            {}

            Expression(ExpressionType t, const string& st, bool tp = false):
                etype(t), type(new const string(st)), complex(tp)
            {}

            virtual ~Expression();

            virtual void print(ostream& str, int i = 0);

            // Expression Type
            const ExpressionType etype;

            // Type
            Pointer<const string> type;

            // Complex structure cant be inside Operators
            // imagine incrementing a for loop. it makes no sens
            const bool           complex;

        #if LLVM_CODEGEN
            virtual llvm::Value* code_gen(Generator& g) = 0;
        #endif
    };

    /// VarExprAST - Expression class for var/in
    /// for posterity
    class MutableVariableExpression : public Expression
    {
        public:
            MutableVariableExpression(
                    const vector<pair<string, Expression*> > &varnames,
                    Expression *body):
              var_names(varnames), body(body), Expression(Type_MutableVariableExpression)
            {}

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

            DEFINE_CODE_GEN

            // TYPE

        string name;
    };

    // lhs + rhs ===> 'operator'(lhs, rhs)
    class BinaryExpression: public Expression
    {
        public:

            BinaryExpression(string op, Expression *lhs, Expression *rhs);

            // 'operator'(lhs, rhs)
            void print(ostream& str, int i = 0);

            // found out the return type of the expression
            const Pointer<const string>& return_type()
            {
                // look up the return type in function of the operator
                // TODO
                // type = associative_table_ret[op + (*lhs.type) + (*rhs.type)]

                //
                type = lhs->type;
                return type;
            }

            DEFINE_CODE_GEN

            string       op;
            Expression *lhs;
            Expression *rhs;

    };

    // represent a calling inside an expression
    // 1. Find the Function*
    // 2. Bind Call::Args to Function::Args
    // 3. Generate the specialized function
    class CallExpression: public Expression
    {
        public:
            typedef vector<Expression*> Arguments;

            CallExpression(const std::string &callee, Arguments& args, char c = '(');

            void print(ostream& str, int i = 0);

            const Pointer<const string>& return_type()
            {
                // find the called function and its return type
                // type = defined_function_lookup[callee + args()].return_type();

                type = Pointer<const string>(new string("double"));
                return type;
            }

            DEFINE_CODE_GEN

            string callee;
            Arguments args;
            // Cal type '(' or '['
            char      ptype;
    };

    // def function(args1, args2, ...) => fn name(args1, args2, ...)
    // argument type will be infered when the function is called
    // I actually don't need to store args type
    // since they will be determined by CallExpression
    class Prototype
    {
        public:

            // with types
//            typedef pair<string, Pointer<const string>&> Parameter;
//            typedef vector<Parameter> Arguments;
            // without types
            typedef vector<string> Arguments;

            Prototype(const string &name, const Arguments& args,
                      bool isoperator = false, unsigned prec = 0);

            void print(ostream& str, int i = 0);

            static void add_param(Arguments& args, const string& arg)
            {
//                args.push_back(Parameter(arg, double_type()));
                args.push_back(arg);
            }

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

            // Type
            const Pointer<const string>& return_type()
            {
                return return_type_t;
            }

            // a function has a type ==> int (*call_back)(double, double);
            //const Pointer<const string> call_type()
            //{}

            // return type
            Pointer<const string> return_type_t;
    };

    // prototype  => fn name(args1, args2, args3, ...)
    // expression =>
    class Function
    {
        public:
            Function(Prototype *proto, Expression *body);

            void print(ostream& str, int i = 0);

            const Pointer<const string>& return_type()
            {
                return prototype->return_type();
            }

            DEFINE_CODE_GEN

            Prototype* prototype;
            Expression* body;
    };

    class IfExpression : public Expression
    {
        public:
            IfExpression(Expression* cond, Expression* then, Expression* lse);

            void print(ostream& str, int i = 0);

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

            const Pointer<const string>& return_type()
            {
                type = operand->type;
                return type;
            }

            DEFINE_CODE_GEN

        string opcode;
        Expression* operand;
    };

    class MultilineExpression : public Expression
    {
        public:

            MultilineExpression():
                Expression(Type_MultilineExpression, true)
            {}

            ~MultilineExpression()
            {}

            virtual void print(ostream& str, int i = 0)
            {
                for(int k = 0; k < expressions.size(); k++)
                {
                    if (expressions[k]->etype == Type_MultilineExpression)
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

    class FloatExpression : public Expression
    {
        public:
        FloatExpression(double value):
            Expression(Type_Float, "double"), value(value)
        {}

        void print(ostream& str, int i = 0)
        {
            str << I(i) << value;
        }

        double value;
    };


    class IntExpression : public Expression
    {
        public:
        IntExpression(int value):
            Expression(Type_Int, "int"), value(value)
        {}

        void print(ostream& str, int i = 0)
        {
            str << I(i) << value;
        }

        int value;
    };

    class StringExpression : public Expression
    {
        public:
        StringExpression(string value):
            Expression(Type_String, "string"), value(value)
        {}

        void print(ostream& str, int i = 0)
        {
            str << I(i) << value;
        }

        string value;
    };

    template<typename T = double>
    class TypedExpression : public Expression
    {
        public:
            TypedExpression(T v):
                Expression(Type_TypedExpression, "auto"), value(v)
            {}

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

    class ClassExpression: public Expression
    {
        public:

            ClassExpression(string class_name):
                Expression(Type_ClassExpression, class_name)
            {
                add_attr("__docstring__", new StringExpression(""));
            }

            ~ClassExpression() {   delete attr("__docstring__");   }

            inline Expression* attr(const Signature& s)  {   return attributes[s];         }
            inline bool    has_attr(const Signature& s)  {   return attributes.count(s);   }
            inline void    add_attr(const Signature& s, Expression* e)   {   attributes[s] = e;  }

            inline Function* method(const Signature& s)   {   return methods[s];  }
            inline bool  has_method(const Signature& s)   {   return methods.count(s);    }
            inline void  add_method(const Signature& s, Function* e)   {   methods[s] = e;    }

            inline const string& name() {   return (*type); }

            void set_docstring(string val) {   ((TypedExpression<string>*) attr("__docstring__"))->value = val;    }

            void print(ostream& str, int i = 0);

            unordered_map<Signature, Expression*> attributes;
            unordered_map<Signature, Function*> methods;
    };

    // wrong way
//    class PyFunction: public ClassExpression
//    {
//        PyFunction(string name):
//            ClassExpression(name)
//        {
//            add_attr("__call__", 0);
//        }

//        ~PyFunction()
//        {
//            Expression* e = attr("__call__");
//            if (e != nullptr)
//                delete e;
//        }


//        // naaaa it is Function
////        CallExpression* caller()
////        {
////            return (CallExpression*) attr("__call__");
////        }
//    };

//    typedef TypedExpression<double> DoubleExpression;
//    typedef TypedExpression<int>    IntExpression;
//    typedef TypedExpression<std::string>    StringExpression;
}

namespace AST = AbstractSyntaxTree;
}

#endif
