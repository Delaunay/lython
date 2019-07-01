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
//#define LYTHON_COMMFUNC(type, body)                                            \
//    virtual type partial_eval() body                                           \
//    virtual type derivate()                                                    \
//        body                                                                   \
//    virtual std::ostream& print(std::ostream &, int indent = 0) body

//#define LYTHON_COMMFUNCCHILD LYTHON_COMMFUNC(ST::Expr, {})

#define LYTHON_KIND(_kind)                                                     \
    Expression::KindExpr kind() override { return _kind; }

#define LYTHON_COMMFUNCCHILD
#define LYTHON_COMMFUNC(...)

namespace lython {

// Private Object
namespace AbstractSyntaxTree {
class Expression {
  public:
    // Explicit RTTI
    enum KindExpr {
        KindPlaceholder,
        KindBinaryOperator,
        KindUnaryOperator,
        KindSeqBlock,
        KindFunction,
        KindUnparsedBlock,
        KindStatement,
        KindValue,
        KindCall,
        KindReference,
        KindStruct
    };

    // this is here but currently no classes are doing dyn-alloc
    // so it is not necessary
    virtual ~Expression();

    LYTHON_COMMFUNC(std::shared_ptr<Expression>, {})

    virtual std::ostream& print(std::ostream &, int32 indent = 0) = 0;
    virtual KindExpr kind() { return KindExpr(-1); }

  private:
};
} // namespace AbstractSyntaxTree
namespace AST = AbstractSyntaxTree;

// Public Object
namespace SyntaxTree {
// I am using shared_ptr because it is the simpliest to handle
// but I may want to change in the future
typedef std::shared_ptr<AST::Expression> Expr;

template <typename T, typename... Args> Expr make_expr(Args &&... args) {
    return Expr(new T(std::forward<Args>(args)...));
}

} // namespace SyntaxTree
namespace ST = SyntaxTree;

namespace AbstractSyntaxTree {
// We declare the leafs of our program
// -----------------------------------

// A Placeholder is a special construct that represent a unknown value
// that is unknown at compile time but will be known at runtime
class Placeholder : public Expression {
  public:
    Placeholder(const std::string &name, const std::string &type)
        : _name(make_name(name)), _type(make_type(type)) {}

    Placeholder(Name name, Type type) : _name(name), _type(type) {}

    Name &name() { return _name; }
    Type &type() { return _type; }

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindPlaceholder)

    ~Placeholder() override;
    std::ostream& print(std::ostream & out, int32 indent = 0) override;

  private:
    Name _name;
    Type _type; // Only used for compile type
                // type info are discarded later
};

// I want placeholder to be hashable
struct pl_hash {
    std::size_t operator()(Placeholder &v) const noexcept;
    std::hash<std::string> _h;
};

typedef std::unordered_map<Placeholder, ST::Expr, pl_hash> Variables;

class Value: public Expression{
public:
    template<typename T>
    Value(T val, Type type):
        _value(new ValueHolder<T>(val)), _type(type){
    }

    LYTHON_KIND(KindValue)

    ~Value() override {
        delete _value;
    }

    std::ostream& print(std::ostream & out, int32 indent = 0) override{
        return _value->print(out, indent);
    }

private:
    struct BaseHolder{
        virtual std::ostream& print(std::ostream & out, int32 indent = 0) = 0;

        virtual ~BaseHolder(){}
    };

    template<typename T>
    struct ValueHolder: public BaseHolder{
        ValueHolder(T val):
            value(val)
        {}

        std::ostream& print(std::ostream & out, int32 = 0) override{
            return out << value;
        }

        T value;
    };

    BaseHolder* _value = nullptr;
    Type _type;
};

// We declare Basic nodes of our program
// -------------------------------------

// A binary operator is a function with two parameters
// Some language specify binary operator as function
// we want our language to be readable
class BinaryOperator : public Expression {
  public:
    BinaryOperator(ST::Expr rhs, ST::Expr lhs, ST::Expr op)
        : _rhs(rhs), _lhs(lhs), _op(op) {}

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindBinaryOperator)

    ~BinaryOperator() override;

    std::ostream& print(std::ostream & out, int32 indent = 0) override;

  private:
    ST::Expr _rhs;
    ST::Expr _lhs;
    ST::Expr _op;
};

class UnaryOperator : public Expression {
  public:
    UnaryOperator() {}

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindUnaryOperator)

    ~UnaryOperator() override;

    std::ostream& print(std::ostream & out, int32 indent = 0) override;

    ST::Expr& expr(){
        return _expr;
    }

    std::string& operation(){
        return _op;
    }

  private:
    ST::Expr _expr;
    std::string _op;
};

class Call : public Expression {
  public:
    typedef std::vector<ST::Expr> Arguments;

    Call() {}

    LYTHON_KIND(KindCall)

    ~Call() override;

    std::ostream& print(std::ostream & out, int32 indent = 0) override;

    ST::Expr& function() {return _function;}
    Arguments& arguments() { return _arguments;}

  private:
    ST::Expr _function;
    Arguments _arguments;
};

// Block Instruction
// -------------------------------------

// Should I make a sequential + Parallel Intruction Block ?
// similar to let and let* in scheme

// Add get_return_type()
class SeqBlock : public Expression {
  public:
    typedef std::vector<ST::Expr> Blocks;

    SeqBlock() {}

    Blocks& blocks() { return _block; }

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindSeqBlock)

    ~SeqBlock() override;

    std::ostream& print(std::ostream & out, int32 indent = 0) override;

  private:
    Blocks _block;
};

// Functions
// -------------------------------------

// Functions are Top level expression
class Function : public Expression {
  public:
    typedef std::vector<Placeholder> Arguments;

    Function(const std::string &name) : _name(make_name(name)) {}

    ST::Expr &body() { return _body; }
    Arguments &args() { return _args; }
    Type &return_type() { return _return_type; }
    Name &name() { return _name; }

    ~Function() override;

    void set_return_type(const std::string &str) {
        _return_type = make_type(str);
    }

    // LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindFunction)

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    std::string& docstring(){
        return _docstring;
    }
  private:
    ST::Expr _body = nullptr;
    Arguments _args;
    Type _return_type;
    Name _name;
    std::string _docstring;
};

//  This allow me to read an entire file but only process
//  used ens
class UnparsedBlock : public Expression {
  public:
    typedef std::vector<Token> Tokens;

    UnparsedBlock() = default;

    UnparsedBlock(Tokens &toks) : _toks(toks) {}

    ~UnparsedBlock() override;

    // LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindUnparsedBlock)

    Tokens &tokens() { return _toks; }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

  private:
    Tokens _toks;
};

class Statement: public Expression{
  public:
    Statement(){}

    LYTHON_KIND(KindStatement)

    int8& statement() {
        return _statement;
    }

    ST::Expr& expr(){
        return _expr;
    }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

  private:
    int8 _statement;
    ST::Expr _expr;
};

class Ref: public Expression{
public:
    Ref() = default;

    LYTHON_KIND(KindReference)

    std::string& name(){
        return _name;
    }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

private:
    std::string _name;
};


class Struct: public Expression{
public:
    typedef std::unordered_map<std::string, ST::Expr> Attributes;

    Struct() = default;

    LYTHON_KIND(KindStruct)

    std::string& name(){
        return _name;
    }

    Attributes& attributes(){
        return _attributes;
    }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    std::string& docstring(){
        return _docstring;
    }

private:
    std::string _name;
    Attributes _attributes;
    std::string _docstring;
};


} // namespace AbstractSyntaxTree
} // namespace lython
