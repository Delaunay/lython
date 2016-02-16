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
#include "Scope.h"

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
 * Memory Management
 * ------------------
 *
 *  ScopedExpression delete their components themselves.
 *  Every other expression should be managed by a ScopedExpression
 *
 */


// BaseExpression
// \-- Statement      if/for/while
// \-- Declaration    class/def/ variable declaration
// \-- Expression     everything else

using namespace std;

namespace LIBNAMESPACE
{
    class Scope;

namespace AbstractSyntaxTree
{
    typedef string Signature;

    #define I(x) std::string(x * 4, ' ')

    // variables/function/class declaration
    // ScopedExpression must have a Signature which is unique for Similar item

    enum ExpressionType
    {
        Type_Expression                 =     0,        // Everything is an expression
        Type_ScopedExpression           =     1,
        Type_ScopeExpression            =     2,
        Type_VariableExpression         =     4 | Type_ScopedExpression,

        Type_BinaryExpression           =     8,
        Type_CallExpression             =    16,
        Type_ForExpression              =    32,
        Type_Function                   =    64 | Type_ScopedExpression,
        Type_IfExpression               =   128,
        Type_MultilineExpression        =   256 | Type_ScopeExpression,
        Type_MutableVariableExpression  =   512,
        Type_Prototype                  =  1024,
        Type_TypedExpression            =  2048,
        Type_UnaryExpression            =  4096,
        Type_ClassExpression            =  8192 | Type_ScopedExpression | Type_ScopeExpression,

        Type_Int                        = 16384 | Type_TypedExpression,
        Type_Float                      = 32768 | Type_TypedExpression,
        Type_String                     = 65536 | Type_TypedExpression
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
    Pointer<const string>& double_type();   // to simplify: ev erything else is double by default

    // generate correct indent
    // string I(unsigned int n);


//    class AbstractExpression
//    {
//        virtual void print(ostream& str, int i = 0){}
//    };

//    class SignatureExpression : public AbstractExpression
//    {
//        typedef std::string Signature;

//        virtual Signature sign();
//    };

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

            //bool is_scoped()    {   return etype | Type_ScopedExpression;    }

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

    /*
     *  Expression inside Scope
     *
     *  Scoped Expression register themselves into a scope
     */
    class ScopedExpression: public Expression
    {
    public:
        ScopedExpression(const std::string& sig, Scope& scope, ExpressionType t = Type_ScopedExpression, bool tp = false):
            Expression(t, tp)
        {
            scope.add_definition(sig, this);
        }

        ScopedExpression(const std::string& sig, Scope& scope, ExpressionType t, const string& st, bool tp = false):
            Expression(t, st, tp)
        {
            scope.add_definition(sig, this);
        }

        virtual const Signature& sign()
        {
            return (*none_type());
        }
    };

    /*
     *  Expression with Scope
     */
    class ScopeExpression: public Expression
    {
    public:
        ScopeExpression(ExpressionType t = Type_ScopeExpression, bool tp = false):
            Expression(t, tp), _scope("", true)
        {}

        Scope& scope()  {   return _scope;   }

    private:
        Scope _scope;
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
    class VariableExpression: public ScopedExpression
    {
        public:
            VariableExpression(Scope &s, const string &name);

            void print(ostream& str, int i = 0);

            const Signature& sign()    {   return name; }

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

//            const Signature& sign()
//            {
//                // Assignment
//                if (op == "="){
//                    if (lhs->etype == Type_VariableExpression){
//                        return ((VariableExpression*) lhs)->sign();
//                    }
//                }

//                return Signature();
//            }

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
    class Function: public ScopedExpression
    {
        public:
            Function(const std::string& n, Scope &s, Prototype *proto, Expression *body);

            void print(ostream& str, int i = 0);

            const Pointer<const string>& return_type()
            {
                return prototype->return_type();
            }

            const Signature& sign() override
            {
//                // We check for assignment
//                if (prototype->name == std::string())
//                {
//                    if (body->etype == Type_BinaryExpression)
//                    {
//                        BinaryExpression* n = (BinaryExpression*) body;

//                        if (n->op == "=")
//                            if (n->lhs->etype == Type_VariableExpression)
//                                return ((VariableExpression*) n->lhs)->name;
//                    }
//                }

                return prototype->name;
            }

            DEFINE_CODE_GEN

            Prototype* prototype;
            Expression* body;
            Scope* scope;
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

    class MultilineExpression : public ScopeExpression
    {
        public:

            MultilineExpression():
                ScopeExpression(Type_MultilineExpression, true)
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

    class ClassExpression: public ScopedExpression
    {
        public:

            ClassExpression(Scope& scpe, string class_name):
                ScopedExpression(class_name, scpe, Type_ClassExpression, class_name),
                _scope("", false)
            {
                add_attr("__docstring__", new StringExpression(""));
            }

            ~ClassExpression()
            {
                for (auto& i:attributes)
                    delete i.second;

                for (auto& i:methods)
                    delete i.second;
            }

            inline Expression* attr(const Signature& s)  {   return attributes[s];         }
            inline bool    has_attr(const Signature& s)  {   return attributes.count(s);   }
            inline void    add_attr(const Signature& s, Expression* e)
            {
                _scope.add_definition(s, e);
                attributes[s] = e;
            }

            inline Function* method(const Signature& s)   {   return methods[s];  }
            inline bool  has_method(const Signature& s)   {   return methods.count(s);    }
            inline void  add_method(const Signature& s, Function* e)
            {
                _scope.add_definition(s, e);
                methods[s] = e;
            }

            inline const string& name() {   return (*type); }

            void set_docstring(string val) {   ((TypedExpression<string>*) attr("__docstring__"))->value = val;    }

            void print(ostream& str, int i = 0);

            Signature& sign()    {   return (Signature &)(*type); }

            unordered_map<Signature, Expression*> attributes;
            unordered_map<Signature, Function*> methods;


            Scope& scope()  {   return _scope;   }

    private:
            Scope _scope;
    };
}

namespace AST = AbstractSyntaxTree;
}

#endif
