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
#include <vector>

#include "../Lexer/Tokens.h"
#include "Names.h"

// declare common function
// I don't have to add them to the class declaration one by one
// not best design but since I am experiencing this might change
// quite a lot
#define LYTHON_COMMFUNC(type, body)\
    virtual type partial_eval() body\
    virtual type derivate() body \
    virtual std::ostream& print(std::ostream&, int indent=0) body

#define LYTHON_COMMFUNCCHILD LYTHON_COMMFUNC(ST::Expr, {})

#define LYTHON_KIND(_kind) Expression::KindExpr kind() { return _kind;}

namespace lython{

// Private Object
namespace AbstractSyntaxTree{
    class Expression
    {
        public:
            // Explicit RTTI
            enum KindExpr{
                KindPlaceholder,
                KindConstant,
                KindBinaryOperator,
                KindUnaryOperator,
                KindSeqBlock,
                KindFunction,
                KindUnparsedBlock
            };

            // this is here but currently no classes are doing dyn-alloc
            // so it is not necessary
            ~Expression(){}

            LYTHON_COMMFUNC(std::shared_ptr<Expression>, {})

            virtual KindExpr kind() { return KindExpr(-1);    }

        private:
    };
}
namespace AST = AbstractSyntaxTree;

// Public Object
namespace SyntaxTree{
    // I am using shared_ptr because it is the simpliest to handle
    // but I may want to change in the future
    typedef std::shared_ptr<AST::Expression> Expr;

    template<typename T, typename... Args>
    Expr make_expr(Args&&... args){
        return Expr(new T(std::forward<Args>(args)...));
    }

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
            LYTHON_KIND(KindPlaceholder)
        
        private:
            Name _name;
            Type _type; // Only used for compile type
                        // type info are discarded later
    };

    // I want placeholder to be hashable
    struct pl_hash{
        std::size_t operator() (Placeholder& v) const noexcept;
        std::hash<std::string> _h;
    };

    typedef std::unordered_map<Placeholder, ST::Expr, pl_hash> Variables;
    
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
            LYTHON_KIND(KindConstant)
            
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
            LYTHON_KIND(KindBinaryOperator)

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
            LYTHON_KIND(KindUnaryOperator)

        private:
            ST::Expr _expr;
            Operator _op;
    };

    // Block Instruction
    // -------------------------------------

    // Should I make a sequential + Parallel Intruction Block ?
    // similar to let and let* in scheme

    // Add get_return_type()
    class SeqBlock: public Expression
    {
    public:
        typedef std::vector<ST::Expr> Block;

        SeqBlock(){}

        Block get_block() { return _block;  }

        LYTHON_COMMFUNCCHILD
        LYTHON_KIND(KindSeqBlock)

    private:
        Block _block;
    };


    // Functions
    // -------------------------------------

    // Functions are Top level expression
    class Function: public Expression{
    public:
        typedef std::vector<Placeholder> Arguments;

        Function(const std::string& name):
            _name(make_name(name))
        {}

        ST::Expr& body()    {   return _body;   }
        Arguments& args()   {   return _args;   }
        Type& return_type() {   return _return_type;    }
        Name& name()        {   return _name;           }

        void set_return_type(const std::string& str){
            _return_type = make_type(str);
        }

        //LYTHON_COMMFUNCCHILD
        LYTHON_KIND(KindFunction)

        std::ostream& print(std::ostream& out, int indent=0){
            out << "def " << *(_name.get()) << "(";

            for(int i = 0, n = _args.size(); i < n; ++i){
                out << *(_args[i].name().get()) << ": " << *(_args[i].type().get());

                if (i < n - 1)
                    out << ", ";
            }

            if(_return_type)
                out << "): -> " << *(_return_type.get()) << "\n";
            else
                out << "):\n";

            _body->print(out, indent + 1);
            return out;
        }

        // Parse Doc String !!

    private:

        ST::Expr  _body;
        Arguments _args;
        Type      _return_type;
        Name      _name;
    };

    //  This allow me to read an entire file but only process
    //  used entities
    class UnparsedBlock: public Expression{
    public:
        typedef std::vector<Token> Tokens;

        UnparsedBlock() = default;

        UnparsedBlock(Tokens& toks):
            _toks(toks)
        {}

        //LYTHON_COMMFUNCCHILD
        LYTHON_KIND(KindUnparsedBlock)

        Tokens& tokens() {   return _toks;   }

        // Parse current expression
        // ST::Expr get_expr()   {}

        std::ostream& print(std::ostream& out, int indent=0){
            for(auto& tok:_toks)
                tok.print(out, indent);
            return out;
        }

    private:
        Tokens _toks;
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
