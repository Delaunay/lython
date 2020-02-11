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

#include "expressions.h"

#include "../lexer/token.h"
#include "../utilities/stack.h"
#include "../interpreter/value.h"
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

//#define LYTHON_COMMFUNCCHILD LYTHON_COMMFUNC(Expression, {})

//     const Expression::NodeKind const kind_tag = _kind;

#define LYTHON_KIND(_kind)                                                     \
   NodeKind kind = NodeKind::_kind;
// NodeKind kind() override { return NodeKind::_kind; }

#define LYTHON_COMMFUNCCHILD
#define LYTHON_COMMFUNC(...)

namespace lython {

class Module;

// Private Object
namespace AST {
struct Node {
public:
    NodeKind kind = NodeKind::KUndefined;

    LYTHON_COMMFUNC(std::shared_ptr<Expression>, {})

    virtual std::ostream &print(std::ostream &, int32 indent = 0) = 0;

    // virtual NodeKind kind() { return NodeKind(-1); }
};

} // namespace AbstractSyntaxTree
namespace AbstractSyntaxTree = AST;
using Attributes = Dict<String, Expression>;


namespace AST {
// We declare the leafs of our program
// -----------------------------------

// A Parameter is a special construct that represent a unknown value
// that is unknown at compile time but will be known at runtime
struct Parameter : public Node {
public:
    LYTHON_KIND(KParameter)

    String     name;
    Expression type;

    Parameter(const String &name, Expression type)
        : name(name), type(type)
    {}

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
};
using ParameterList = Array<Parameter>;

// I want placeholder to be hashable
struct pl_hash {
    std::size_t operator()(Parameter &v) const noexcept;
    std::hash<String> _h;
};

struct Builtin : public Node {
public:
    LYTHON_KIND(KBuiltin)

    String     name;
    Expression type;
    size_t     argument_size;

    Builtin(String name, Expression type, size_t n) :
        name(std::move(name)), type(std::move(type)), argument_size(n)
    {}

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
};

struct Arrow : public Node {
public:
    LYTHON_KIND(KArrow)

    ParameterList params;
    Expression return_type;

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
        out << return_type;

        return_type.print(out, indent);
        return out;
    }
};

using Variables = Dict<Parameter, Expression, pl_hash>;

struct Type : public Node {
public:
    LYTHON_KIND(KType)

    String name;

    Type(String name) : name(std::move(name)) {}

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
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
    Expression ref;
    String name = "";
};

/**
 * instead of creating a billions expression node we create a single node
 *  that holds all the expressions.
 */
struct ReversePolishExpression : public Node {
public:
    ReversePolishExpression(Stack<MathNode> str) : stack(std::move(str)) {}

    LYTHON_KIND(KReversePolish)

    Stack<MathNode> stack;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    String to_infix(Stack<MathNode>::Iterator &iter, int prev = 0);
};

struct ValueExpr : public Node {
public:
    LYTHON_KIND(KValue)

    Value       value;
    Expression  type;

    template <typename T>
    ValueExpr(T val, Expression type) :value(val), type(type) {}

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    template<typename V>
    V get_value(){
        return value.get<V>();
    }

    VTag get_tag(){
        return value.tag;
    }

    template<typename V, typename T>
    T cast(){
        return T(get_value<V>());
    }
};


// We declare Basic nodes of our program
// -------------------------------------

// A binary operator is a function with two parameters
// Some language specify binary operator as function
// we want our language to be readable
struct BinaryOperator : public Node {
public:
    LYTHON_KIND(KBinaryOperator)

    Expression rhs;
    Expression lhs;
    Expression op;

    BinaryOperator(Expression rhs, Expression lhs, Expression op)
        : rhs(std::move(rhs)), lhs(std::move(lhs)), op(std::move(op)) {}

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
};

struct UnaryOperator : public Node {
public:
    LYTHON_KIND(KUnaryOperator)

    Expression expr;
    String op;

    UnaryOperator() = default;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
};

struct Call : public Node {
public:
    using Arguments = Array<Expression>;
    LYTHON_KIND(KCall)

    Expression function;
    Arguments  arguments;

    Call() = default;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
};

// Block Instruction
// -------------------------------------

// Should I make a sequential + Parallel Intruction Block ?
// similar to let and let* in scheme

// Add get_return_type()
struct SeqBlock : public Node {
public:
    using Blocks = Array<Expression>;
    Blocks blocks;

    LYTHON_KIND(KSeqBlock)

    SeqBlock() = default;

    std::ostream &print(std::ostream &out, int32 indent = 0) override; 
};

// Functions
// -------------------------------------

// Functions are Top level expression
struct Function : public Node {
public:
    LYTHON_KIND(KFunction)

    Expression    body;
    ParameterList args;
    Expression    return_type;
    String        name;
    String        docstring;

    Function(String const &name, bool is_extern = false)
        : externed(is_extern), name(make_name(name)) {}

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    bool externed = false;

    // First Entry is the function itself (for recursion)   (also StackFrame unwinding)
    // Then arguments
    // Then global variable access (include calls to outside functions)
    // Finaly return value
    // The frame is created by the AccessTracker inside Module during parsing
    // The frame is used by the evaluator/interpreter to initialize an eval environment
    // for the function once initialized the function should be able to run without side effects
    Array<Tuple<String, int>> frame;
};

//  This allow me to read an entire file but only process
//  used ens
struct UnparsedBlock : public Node {
public:
    using Tokens = Array<Token>;

    LYTHON_KIND(KUnparsedBlock)

    Tokens tokens;

    UnparsedBlock() = default;

    UnparsedBlock(Tokens &toks) : tokens(toks) {}

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
};

struct QualifiedType : public Node {
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

struct Statement : public Node {
public:
    LYTHON_KIND(KStatement)

    int8        statement;
    Expression  expr;

    Statement() = default;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
};

struct Ref : public Node {
public:
    LYTHON_KIND(KReference)

    String      name;
    Expression  type;
    int         index;
    int         length;

    Ref(String name, int loc, int length, Expression type):
        name(std::move(name)), type(type), index(loc), length(length)
    {}

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
};

struct Struct : public Node {
public:
    LYTHON_KIND(KStruct)

    String      name;
    Attributes  attributes;
    String      docstring;

    Struct() = default;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;
};
} // namespace AbstractSyntaxTree
} // namespace lython

#endif
