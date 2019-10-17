#ifndef LYTHON_AST
#define LYTHON_AST
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
#include <numeric>
#include <utility>

#include "../lexer/token.h"
#include "../utilities/stack.h"
#include "names.h"

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

class Module;

// Private Object
namespace AbstractSyntaxTree {
class Expression {
  public:
    // Explicit RTTI
    enum KindExpr {
        KindArrow,
        KindBuiltin,
        KindParameter,
        KindBinaryOperator,
        KindUnaryOperator,
        KindSeqBlock,
        KindFunction,
        KindUnparsedBlock,
        KindStatement,
        KindValue,
        KindCall,
        KindReference,
        KindStruct,
        KindType,
        KindReversePolish,
        KindExternFunction
    };

    // this is here but currently no classes are doing dyn-alloc
    // so it is not necessary
    virtual ~Expression();

    LYTHON_COMMFUNC(std::shared_ptr<Expression>, {})

    virtual std::ostream &print(std::ostream &, int32 indent = 0) = 0;
    virtual KindExpr kind() { return KindExpr(-1); }

    // Module *module() { return _module; }

  private:
    // Module *_module; //! Module this expression belong to
};
} // namespace AbstractSyntaxTree
namespace AST = AbstractSyntaxTree;

// Public Object
namespace SyntaxTree {
// I am using shared_ptr because it is the simpliest to handle
// but I may want to change in the future
using Expr = std::shared_ptr<AST::Expression>;

template <typename T, typename... Args> Expr make_expr(Args &&... args) {
    return Expr(new T(std::forward<Args>(args)...));
}

} // namespace SyntaxTree
namespace ST = SyntaxTree;
using Attributes = Dict<String, ST::Expr>;

namespace AbstractSyntaxTree {
// We declare the leafs of our program
// -----------------------------------

// A Parameter is a special construct that represent a unknown value
// that is unknown at compile time but will be known at runtime
class Parameter : public Expression {
  public:
    Parameter(const String &name, ST::Expr type)
        : _name(make_name(name)), _type(std::move(type)) {}

    Parameter(Name name, ST::Expr type) : _name(name), _type(std::move(type)) {}

    String &name() { return _name; }
    ST::Expr &type() { return _type; }

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindParameter)

    ~Parameter() override;
    std::ostream &print(std::ostream &out, int32 indent = 0) override;

  private:
    String _name;
    ST::Expr _type; // Only used for compile type
                    // type info are discarded later
};
using ParameterList = Array<Parameter>;

// I want placeholder to be hashable
struct pl_hash {
    std::size_t operator()(Parameter &v) const noexcept;
    std::hash<String> _h;
};

class Builtin : public Expression {
  public:
    Builtin(String name, ST::Expr type) : name(std::move(name)), type(std::move(type)) {}

    LYTHON_KIND(KindBuiltin)

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    String name;
    ST::Expr type;
};

class Arrow : public Expression {
  public:
    LYTHON_KIND(KindArrow)

    ParameterList params;
    ST::Expr return_type;

    std::ostream &print(std::ostream &out, int32 indent = 0) override {
        int n = int(params.size()) - 1;

        out << "(";

        for (int i = 0; i < n; ++i) {
            params[size_t(i)].print(out, indent);
            out << ", ";
        }

        if (n >= 0) {
            params[size_t(n)].print(out, indent);
        }

        out << ") -> ";
        return_type->print(out, indent);
        return out;
    }
};

using Variables = Dict<Parameter, ST::Expr, pl_hash>;

class Type : public Expression {
  public:
    Type(String name) : name(std::move(name)) {}

    LYTHON_KIND(KindType)

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    String name;
};

// Math Nodes for ReverPolish parsing
enum class MathKind {
    Operator,
    Value,
    Function,
    VarRef,
    None
};

struct MathNode {
    MathKind kind;
    int arg_count = 1;
    ST::Expr ref = nullptr;
    String name = "";
};

/**
 * instead of creating a billions expression node we create a single node
 *  that holds all the expressions.
 */
class ReversePolishExpression : public Expression {
  public:
    ReversePolishExpression(Stack<MathNode> str) : stack(std::move(str)) {}

    LYTHON_KIND(KindReversePolish)

    Stack<MathNode> stack;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    String to_infix(Stack<MathNode>::Iterator &iter, int prev = 0);
};

class Value : public Expression {
  public:
    template <typename T>
    Value(T val, ST::Expr type) : _value(new ValueHolder<T>(val)), _type(std::move(type)) {}

    LYTHON_KIND(KindValue)

    ~Value() override { delete _value; }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

  private:
    struct BaseHolder {
        virtual std::ostream &print(std::ostream &out, int32 indent = 0) = 0;

        virtual ~BaseHolder() = default;
    };

    template <typename T> struct ValueHolder : public BaseHolder {
        ValueHolder(T val) : value(val) {}

        std::ostream &print(std::ostream &out, int32 = 0) override {
            return out << value;
        }

        T value;
    };

    BaseHolder *_value = nullptr;
    ST::Expr _type;
};

// We declare Basic nodes of our program
// -------------------------------------

// A binary operator is a function with two parameters
// Some language specify binary operator as function
// we want our language to be readable
class BinaryOperator : public Expression {
  public:
    BinaryOperator(ST::Expr rhs, ST::Expr lhs, ST::Expr op)
        : _rhs(std::move(rhs)), _lhs(std::move(lhs)), _op(std::move(op)) {}

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindBinaryOperator)

    ~BinaryOperator() override;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

  private:
    ST::Expr _rhs;
    ST::Expr _lhs;
    ST::Expr _op;
};

class UnaryOperator : public Expression {
  public:
    UnaryOperator() = default;

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindUnaryOperator)

    ~UnaryOperator() override;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    ST::Expr &expr() { return _expr; }

    String &operation() { return _op; }

  private:
    ST::Expr _expr;
    String _op;
};

class Call : public Expression {
  public:
    using Arguments = Array<ST::Expr>;

    Call() = default;

    LYTHON_KIND(KindCall)

    ~Call() override;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    ST::Expr &function() { return _function; }
    Arguments &arguments() { return _arguments; }

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
    using Blocks = std::vector<ST::Expr>;

    SeqBlock() = default;

    Blocks &blocks() { return _block; }

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindSeqBlock)

    ~SeqBlock() override;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

  private:
    Blocks _block;
};

// Functions
// -------------------------------------

// Functions are Top level expression
class Function : public Expression {
  public:
    Function(String const &name, bool is_extern = false)
        : externed(is_extern), _name(make_name(name)) {}

    ST::Expr &body() { return _body; }
    ParameterList &args() { return _args; }
    ST::Expr &return_type() { return _return_type; }
    String &name() { return _name; }

    ~Function() override;

    // LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KindFunction)

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    String &docstring() { return _docstring; }

    bool externed = false;
  private:
    ST::Expr _body = nullptr;
    ParameterList _args;
    ST::Expr _return_type;
    String _name;
    String _docstring;
};

//  This allow me to read an entire file but only process
//  used ens
class UnparsedBlock : public Expression {
  public:
    using Tokens = std::vector<Token>;

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

class QualifiedType : public Expression {
  public:
    enum TypeSpecifier {
        Void,
        Char,
        Short,
        Int,
        Long,
        Float,
        Double,
        Signed,
        Unsigned,
        UserStruct,
        UserEnum,
        UserUnion,
        UserTypedef
    };
    enum StorageSpecifier { Auto, Register, Static, Extern, Typedef };
    enum TypeQualifier { Const, Volatile };

    String name;
    TypeSpecifier spec_type;
    StorageSpecifier spec_storage;
    TypeQualifier type_qualifier;
};

class Statement : public Expression {
  public:
    Statement() = default;

    LYTHON_KIND(KindStatement)

    int8 &statement() { return _statement; }

    ST::Expr &expr() { return _expr; }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

  private:
    int8 _statement;
    ST::Expr _expr;
};

class Ref : public Expression {
  public:
    Ref(String name, int loc):
        _name(std::move(name)), _index(loc)
    {}

    LYTHON_KIND(KindReference)

    String &name() { return _name; }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

  private:
    String _name;
    int    _index;
};

class Struct : public Expression {
  public:
    Struct() = default;

    LYTHON_KIND(KindStruct)

    String &name() { return _name; }

    Attributes &attributes() { return _attributes; }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    String &docstring() { return _docstring; }

  private:
    String      _name;
    Attributes  _attributes;
    String      _docstring;
};

} // namespace AbstractSyntaxTree
} // namespace lython

#endif
