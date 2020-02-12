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

#define LYTHON_COMMFUNCCHILD
#define LYTHON_COMMFUNC(...)

namespace lython {

class Module;

// Private Object
namespace AST {
struct Node {
public:
    NodeKind kind;

    Node(NodeKind k = NodeKind::KUndefined):
        kind(k)
    {}
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
    String     name;
    Expression type;

    Parameter(const String &name, Expression type)
        : Node(NodeKind::KParameter), name(name), type(type)
    {}
};
using ParameterList = Array<Parameter>;

// I want placeholder to be hashable
struct pl_hash {
    std::size_t operator()(Parameter &v) const noexcept;
    std::hash<String> _h;
};

struct Builtin : public Node {
public:
    String     name;
    Expression type;
    size_t     argument_size;

    Builtin(String name, Expression type, size_t n) :
        Node(NodeKind::KBuiltin), name(std::move(name)), type(std::move(type)), argument_size(n)
    {}
};

struct Arrow : public Node {
public:
    Arrow(): Node(NodeKind::KArrow)
    {}

    ParameterList params;
    Expression return_type;
};

using Variables = Dict<Parameter, Expression, pl_hash>;

struct Type : public Node {
public:
    String name;

    Type(String name) : Node(NodeKind::KType), name(std::move(name)) {}
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
struct ReversePolish: public Node {
public:
    Stack<MathNode> stack;

    ReversePolish(Stack<MathNode> str)
        : Node(NodeKind::KReversePolish), stack(std::move(str)) {}
};

struct Value : public Node {
public:
    lython::Value value;
    Expression    type;

    template <typename T>
    Value(T val, Expression type)
        : Node(NodeKind::KValue), value(val), type(type) {}

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
    Expression rhs;
    Expression lhs;
    Expression op;

    BinaryOperator(Expression rhs, Expression lhs, Expression op)
        : Node(NodeKind::KBinaryOperator), rhs(std::move(rhs)), lhs(std::move(lhs)), op(std::move(op)) {}
};

struct UnaryOperator : public Node {
public:
    Expression expr;
    String op;

    UnaryOperator():
        Node(NodeKind::KUnaryOperator)
    {}
};

struct Call : public Node {
public:
    using Arguments = Array<Expression>;

    Expression function;
    Arguments  arguments;

    Call():
        Node(NodeKind::KCall)
    {}
};

// Block Instruction
// -------------------------------------

// Should I make a sequential + Parallel Intruction Block ?
// similar to let and let* in scheme

// Add get_return_type()
struct SeqBlock : public Node {
public:
    Array<Expression> blocks;

    SeqBlock():
        Node(NodeKind::KSeqBlock)
    {}
};

// Functions
// -------------------------------------

// Functions are Top level expression
struct Function : public Node {
public:
    Expression    body;
    ParameterList args;
    Expression    return_type;
    String        name;
    String        docstring;

    Function(String const &name)
        : Node(NodeKind::KFunction), name(make_name(name))
    {}
};

struct ExternFunction: public Node{
    String name;

    ExternFunction(String const &name)
        : Node(NodeKind::KExternFunction), name(make_name(name))
    {}
};

//  This allow me to read an entire file but only process
//  used ens
struct UnparsedBlock : public Node {
public:
    Array<Token> tokens;

    UnparsedBlock() = default;

    UnparsedBlock(Array<Token> &toks)
        : Node(NodeKind::KUnparsedBlock), tokens(toks)
    {}
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
    int8        statement;
    Expression  expr;

    Statement(): Node(NodeKind::KStatement)
    {}
};

struct Reference : public Node {
public:
    String      name;
    Expression  type;
    int         index;
    int         length;

    Reference(String name, int loc, int length, Expression type):
        Node(NodeKind::KReference), name(std::move(name)), type(type), index(loc), length(length)
    {}
};
using Ref = Reference;

struct Struct : public Node {
public:
    String      name;
    Attributes  attributes;
    String      docstring;

    Struct():
        Node(NodeKind::KStruct)
    {}
};
} // namespace AbstractSyntaxTree
} // namespace lython

#endif
