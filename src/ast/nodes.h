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

#include "ast/expressions.h"
#include "ast/names.h"
#include "lexer/token.h"
#include "utilities/stack.h"
#include "interpreter/value.h"
#include "parser/module.h"

namespace lython {

namespace AST {
struct Node {
public:
    NodeKind kind;
    Token    start = dummy();
    Token    end   = dummy();

    Node(NodeKind k = NodeKind::KUndefined):
        kind(k)
    {}
};

} // namespace AbstractSyntaxTree
namespace AbstractSyntaxTree = AST;
using Attributes = Array<Tuple<StringRef, Expression>>;


namespace AST {
// We declare the leafs of our program
// -----------------------------------

// A Parameter is a special construct that represent a unknown value
// that is unknown at compile time but will be known at runtime
struct Parameter : public Node {
public:
    StringRef  name;
    Expression type;

    Parameter(StringRef name, Expression type)
        : Node(NodeKind::KParameter), name(name), type(type)
    {}

    Parameter(const String &name, Expression type)
        : Parameter(get_string(name), type)
    {}
};

struct pl_hash {
    std::size_t operator()(Parameter &v) const noexcept;
    std::hash<std::size_t> _h;
};


using ParameterList = Array<Parameter>;
using ParameterDict = Dict<StringRef, Parameter, string_ref_hash>;


struct Builtin : public Node {
public:
    StringRef  name;
    Expression type;
    size_t     argument_size;

    Builtin(StringRef name, Expression type, size_t n) :
        Node(NodeKind::KBuiltin), name(name), type(std::move(type)), argument_size(n)
    {}

    Builtin(const String &name, Expression type, size_t n)
        : Builtin(get_string(name), type, n)
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

    ValueKind get_tag(){
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
    StringRef op;
    int precedence;

    BinaryOperator(Expression lhs, Expression rhs, StringRef op, int precedence=0)
        : Node(NodeKind::KBinaryOperator), rhs(std::move(rhs)), lhs(std::move(lhs)), op(op), precedence(precedence) {}
};

struct UnaryOperator : public Node {
public:
    Expression expr;
    StringRef op;
    int precedence;

    UnaryOperator(StringRef op, Expression expr, int precedence=0):
        Node(NodeKind::KUnaryOperator), expr(expr), op(op), precedence(precedence)
    {}

    UnaryOperator(String const& op, Expression expr, int precedence=0):
        UnaryOperator(get_string(op), expr, precedence)
    {}
};

struct Operator : public Node {
public:
    String name;

    Operator(String op)
        : Node(NodeKind::KOperator), name(std::move(op)) {}
};


struct Call : public Node {
public:
    using Arguments = Array<Expression>;
    using KwArguments = Dict<String, Expression>;

    Expression function;
    // Positional arguments
    Arguments   arguments;
    // Keyword arguments
    KwArguments kwargs;

    Call():
        Node(NodeKind::KCall)
    {}
};

// from <path> import <export_name> [as <import_name>], ...
// import <path> [as <name>]
struct Import: public Node{
    struct DeclarationImport{
        DeclarationImport(StringRef exp, StringRef imp):
            export_name(exp), import_name(imp)
        {}

        DeclarationImport(String const& exp, String const& imp):
            DeclarationImport(get_string(exp), get_string(imp))
        {}

        StringRef export_name;
        StringRef import_name;
    };

    using PackagePath = Array<StringRef>;
    using DeclarationImports = Array<DeclarationImport>;

    PackagePath        path;        // package.package.package
    StringRef          name;        // as <name>
    DeclarationImports imports;     // *(<export_name> [as <import_name>])
    // parsed module resulting of the import
    Module             module;

    Import():
        Node(NodeKind::KImport)
    {}

    String module_path() const {
        return join(".", path);
    }

    String file_path() const {
        return join("/", path);
    }
};

struct ImportedExpr: public Node{
    ImportedExpr(Expression imp, Expression ref):
        Node(NodeKind::KImportedExpr), import(imp), ref(ref)
    {}

    ImportedExpr(Expression imp, StringRef ref):
        Node(NodeKind::KImportedExpr), import(imp), name(ref)
    {}

    Expression import;  // package that is imported
    Expression ref;     // reference to the imported module
    StringRef  name;
};


/*
dict.get('key') match:
    case None:
        raise RuntimeError

    case (a, b, c):
        return a + b + c

    case (a, b) where a > 2:
        return a + b

    except ValueError:
        pass

    case Value(a, b) where a > 2:
        return a + b

    except:
        pass

    default:
        pass
 */
struct Match: public Node{
    struct Pattern{};

    struct Branch{
        Pattern     pat;
        Expression  exec;
        // To avoid nesting too many expression together we could attach
        // an except to every Branch
        // Expression except;
    };

    using Branches = Array<Branch>;

    Expression target;          // match <target>:
    Branches   branches;        // case <pattern>: <expression>
    Expression default_branch;  // default: <expression>

    Match():
        Node(NodeKind::KMatch)
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
    StringRef     name;
    String        docstring;

    Function(StringRef name)
        : Node(NodeKind::KFunction), name(name)
    {}

    Function(String const &name)
        : Function(get_string(name))
    {}
};

struct ExternFunction: public Node{
    StringRef name;

    ExternFunction(String const &name)
        : Node(NodeKind::KExternFunction), name(get_string(name))
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

struct Statement : public Node {
public:
    int8        statement;
    Expression  expr;

    Statement(): Node(NodeKind::KStatement)
    {}
};

struct Reference : public Node {
public:
    StringRef   name;
    Expression  type;
    int         index;
    int         length;

    Reference(StringRef name, int loc, int length, Expression type):
        Node(NodeKind::KReference), name(name), type(type), index(loc), length(length)
    {}

    Reference(String const& name, int loc, int length, Expression type):
        Reference(get_string(name), loc, length, type)
    {}
};
using Ref = Reference;

struct Struct : public Node {
public:
    using IndexMapping = Dict<StringRef, int, string_ref_hash>;

    StringRef    name;
    Attributes   attributes;  // Ordered list of attributes
    IndexMapping offset;      // String to int
    String       docstring;

    Struct(StringRef name):
        Node(NodeKind::KStruct), name(name)
    {}

    Struct(String const& name):
        Struct(get_string(name))
    {}

    void insert(String const& attr, Expression expr){
        return insert(get_string(attr), expr);
    }

    void insert(StringRef const& attr, Expression expr){
        offset[attr] = int(attributes.size());
        attributes.emplace_back(attr, expr);
    }
};

} // namespace AbstractSyntaxTree
} // namespace lython

#endif
