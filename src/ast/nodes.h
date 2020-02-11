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
    NodeKind kind() override { return NodeKind::_kind; }

#define LYTHON_COMMFUNCCHILD
#define LYTHON_COMMFUNC(...)

namespace lython {

class Module;

// Private Object
namespace AST {
class Node {
public:
    virtual ~Node();

    LYTHON_COMMFUNC(std::shared_ptr<Expression>, {})

    virtual std::ostream &print(std::ostream &, int32 indent = 0) = 0;

    virtual NodeKind kind() { return NodeKind(-1); }
};

} // namespace AbstractSyntaxTree
namespace AbstractSyntaxTree = AST;
using Attributes = Dict<String, Expression>;


namespace AST {
// We declare the leafs of our program
// -----------------------------------

// A Parameter is a special construct that represent a unknown value
// that is unknown at compile time but will be known at runtime
class Parameter : public Node {
public:
    Parameter(const String &name, Expression type)
        : _name(make_name(name)), _type(std::move(type)) {}

    Parameter(Name name, Expression type) : _name(name), _type(std::move(type)) {}

    String const &name() const { return _name; }
    String  &name()  { return _name; }
    Expression &type() { return _type; }

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KParameter)

    ~Parameter() override;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

private:
    String _name;
    Expression _type; // Only used for compile type
        // type info are discarded later
};
using ParameterList = Array<Parameter>;

// I want placeholder to be hashable
struct pl_hash {
    std::size_t operator()(Parameter &v) const noexcept;
    std::hash<String> _h;
};

class Builtin : public Node {
public:
    Builtin(String name, Expression type, size_t n) :
        name(std::move(name)), type(std::move(type)), argument_size(n)
    {}

    LYTHON_KIND(KBuiltin)

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    String name;
    Expression type;
    size_t argument_size;
};

class Arrow : public Node {
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

class Type : public Node {
public:
    Type(String name) : name(std::move(name)) {}

    LYTHON_KIND(KType)

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
    Expression ref;
    String name = "";
};

/**
 * instead of creating a billions expression node we create a single node
 *  that holds all the expressions.
 */
class ReversePolishExpression : public Node {
public:
    ReversePolishExpression(Stack<MathNode> str) : stack(std::move(str)) {}

    LYTHON_KIND(KReversePolish)

    Stack<MathNode> stack;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    String to_infix(Stack<MathNode>::Iterator &iter, int prev = 0);
};

class ValueExpr : public Node {
public:
    LYTHON_KIND(KValue)

    template <typename T>
    ValueExpr(T val, Expression type) :value(val), _type(std::move(type)) {}

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

    Value value;
    Expression _type;
};


// We declare Basic nodes of our program
// -------------------------------------

// A binary operator is a function with two parameters
// Some language specify binary operator as function
// we want our language to be readable
class BinaryOperator : public Node {
public:
    BinaryOperator(Expression rhs, Expression lhs, Expression op)
        : _rhs(std::move(rhs)), _lhs(std::move(lhs)), _op(std::move(op)) {}

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KBinaryOperator)

    ~BinaryOperator() override;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

private:
    Expression _rhs;
    Expression _lhs;
    Expression _op;
};

class UnaryOperator : public Node {
public:
    UnaryOperator() = default;

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KUnaryOperator)

    ~UnaryOperator() override;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    Expression &expr() { return _expr; }

    String &operation() { return _op; }

private:
    Expression _expr;
    String _op;
};

class Call : public Node {
public:
    using Arguments = Array<Expression>;

    Call() = default;

    LYTHON_KIND(KCall)

    ~Call() override;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    Expression &function() { return _function; }
    Arguments &arguments() { return _arguments; }

private:
    Expression _function;
    Arguments _arguments;
};

// Block Instruction
// -------------------------------------

// Should I make a sequential + Parallel Intruction Block ?
// similar to let and let* in scheme

// Add get_return_type()
class SeqBlock : public Node {
public:
    using Blocks = std::vector<Expression>;

    SeqBlock() = default;

    Blocks &blocks() { return _block; }

    LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KSeqBlock)

    ~SeqBlock() override;

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

private:
    Blocks _block;
};

// Functions
// -------------------------------------

// Functions are Top level expression
class Function : public Node {
public:
    Function(String const &name, bool is_extern = false)
        : externed(is_extern), _name(make_name(name)) {}

    Expression &body() { return _body; }
    ParameterList &args() { return _args; }
    Expression &return_type() { return _return_type; }
    String const &name() const { return _name; }
    String &name() { return _name; }

    ~Function() override;

    // LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KFunction)

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

    String &docstring() { return _docstring; }

    bool externed = false;

    // First Entry is the function itself (for recursion)   (also StackFrame unwinding)
    // Then arguments
    // Then global variable access (include calls to outside functions)
    // Finaly return value
    // The frame is created by the AccessTracker inside Module during parsing
    // The frame is used by the evaluator/interpreter to initialize an eval environment
    // for the function once initialized the function should be able to run without side effects
    Array<Tuple<String, int>> frame;

private:
    Expression    _body;
    ParameterList _args;
    Expression    _return_type;
    String        _name;
    String        _docstring;
};

//  This allow me to read an entire file but only process
//  used ens
class UnparsedBlock : public Node {
public:
    using Tokens = std::vector<Token>;

    UnparsedBlock() = default;

    UnparsedBlock(Tokens &toks) : _toks(toks) {}

    ~UnparsedBlock() override;

    // LYTHON_COMMFUNCCHILD
    LYTHON_KIND(KUnparsedBlock)

    Tokens &tokens() { return _toks; }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

private:
    Tokens _toks;
};

class QualifiedType : public Node {
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

class Statement : public Node {
public:
    Statement() = default;

    LYTHON_KIND(KStatement)

    int8 &statement() { return _statement; }

    Expression &expr() { return _expr; }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

private:
    int8 _statement;
    Expression _expr;
};

class Ref : public Node {
public:
    Ref(String name, int loc, int length, Expression type):
        _name(std::move(name)), _type(type), _index(loc), _length(length)
    {}

    LYTHON_KIND(KReference)

    String &name() { return _name; }
    int index() const { return _index; }
    int length() const { return _length; }

    std::ostream &print(std::ostream &out, int32 indent = 0) override;

private:
    String _name;
    Expression _type;
    int    _index;
    int    _length;
};

class Struct : public Node {
public:
    Struct() = default;

    LYTHON_KIND(KStruct)

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
