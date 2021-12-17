#ifndef LYTHON_SEXPR_HEADER
#define LYTHON_SEXPR_HEADER

#include <memory>

#include "../dtypes.h"
#include "ast/nodekind.h"
#include "constant.h"

#include "logging/logging.h"
#include "utilities/names.h"
#include "utilities/object.h"
#include "utilities/optional.h"

namespace lython {

using Identifier = StringRef;

String str(NodeKind k);

enum class NodeFamily : int8_t
{
    Module,
    Statement,
    Expression,
    Pattern,
};

// col_offset is the byte offset in the utf8 string the parser uses
struct CommonAttributes {
    int           lineno;
    int           col_offset;
    Optional<int> end_lineno;
    Optional<int> end_col_offset;
};

template <typename T>
struct NodeTrait {
    enum Constants
    { kind = int(NodeKind::Invalid) };
};

template <typename T>
NodeKind nodekind() {
    return NodeKind(NodeTrait<T>::kind);
}

struct Node: public GCObject {
    // I think only statements need the indentaion
    virtual void print(std::ostream &out, int indent = 0) const = 0;

    String __str__() const;

    Node(NodeKind _kind): kind(_kind) {}

    virtual NodeFamily family() const = 0;

    const NodeKind kind;

    template <typename T>
    bool is_instance() const {
        return kind == nodekind<T>();
    }
};

struct ModNode: public Node {
    ModNode(NodeKind kind): Node(kind) {}

    NodeFamily family() const override { return NodeFamily::Module; }
};

struct StmtNode: public CommonAttributes, public Node {
    StmtNode(NodeKind kind): Node(kind) {}

    NodeFamily family() const override { return NodeFamily::Statement; }
};

struct ExprNode: public CommonAttributes, public Node {
    ExprNode(NodeKind kind): Node(kind) {}

    NodeFamily family() const override { return NodeFamily::Expression; }
};

enum class ConversionKind : int8_t
{
    None           = -1,
    String         = 115,
    Representation = 114,
    ASCII          = 97
};

enum class BinaryOperator : int8_t
{
    None,
    Add,
    Sub,
    Mult,
    MatMult,
    Div,
    Mod,
    Pow,
    LShift,
    RShift,
    BitOr,
    BitXor,
    BitAnd,
    FloorDiv,

    EltMult,
    EltDiv
};

void print_op(std::ostream &out, BinaryOperator op, bool aug = false);

enum class BoolOperator : int8_t
{
    None,
    And,
    Or
};

void print_op(std::ostream &out, BoolOperator op);

enum class UnaryOperator : int8_t
{
    None,
    Invert, // ~
    Not,    // !
    UAdd,   // +
    USub,   // -
};

void print_op(std::ostream &out, UnaryOperator op);

enum class ExprContext : int8_t
{
    Load,
    Store,
    Del
};

enum class CmpOperator : int8_t
{
    None,
    Eq,
    NotEq,
    Lt,
    LtE,
    Gt,
    GtE,
    Is,
    IsNot,
    In,
    NotIn,
};

void print_op(std::ostream &out, CmpOperator op);

struct Comprehension {
    ExprNode *        target = nullptr;
    ExprNode *        iter   = nullptr;
    Array<ExprNode *> ifs;
    int               is_async : 1;

    String __str__() const;

    void print(std::ostream &out, int indent) const;
};

void print(std::ostream &out, int indent, Array<StmtNode *> const &body);

struct ExceptHandler: public CommonAttributes {
    Optional<ExprNode *> type;
    Optional<Identifier> name;
    Array<StmtNode *>    body;

    void print(std::ostream &out, int indent) const;
};

struct Arg: public CommonAttributes {
    Identifier           arg;
    Optional<ExprNode *> annotation;
    Optional<String>     type_comment;

    void print(std::ostream &out, int indent = 0) const;
};

struct Arguments {
    Arguments() = default;

    Arguments(Arguments const &) = default;

    Array<Arg>        posonlyargs;
    Array<Arg>        args;
    Optional<Arg>     vararg; // *args
    Array<Arg>        kwonlyargs;
    Array<ExprNode *> kw_defaults;
    Optional<Arg>     kwarg; // **kwargs
    Array<ExprNode *> defaults;

    void print(std::ostream &out, int indent) const;
};

struct Keyword: public CommonAttributes {
    Identifier arg; // why is this optional ?
                    // it is marked as optional in the python AST
    ExprNode *value = nullptr;

    void print(std::ostream &out, int indent) const;
};

struct Alias {
    Identifier           name;
    Optional<Identifier> asname;

    void print(std::ostream &out, int indent) const;
};

struct WithItem {
    ExprNode *           context_expr = nullptr;
    Optional<ExprNode *> optional_vars;

    void print(std::ostream &out, int indent) const;
};

struct TypeIgnore {
    int    lineno;
    String tag;
};

struct Pattern: public CommonAttributes, public Node {
    Pattern(NodeKind kind): Node(kind) {}

    void print(std::ostream &out, int idt) const override final { print(out); }

    virtual void print(std::ostream &out) const = 0;

    NodeFamily family() const override { return NodeFamily::Pattern; }

    String __str__() const;
};

struct MatchValue: public Pattern {
    ExprNode *value;

    void print(std::ostream &out) const override;

    MatchValue(): Pattern(NodeKind::MatchValue) {}
};

struct MatchSingleton: public Pattern {
    ConstantValue value;

    void print(std::ostream &out) const override;

    MatchSingleton(): Pattern(NodeKind::MatchSingleton) {}
};

struct MatchSequence: public Pattern {
    Array<Pattern *> patterns;

    void print(std::ostream &out) const override;

    MatchSequence(): Pattern(NodeKind::MatchSequence) {}
};

// The optional "rest" MatchMapping parameter handles capturing extra mapping keys
struct MatchMapping: public Pattern {
    Array<ExprNode *>    keys;
    Array<Pattern *>     patterns;
    Optional<Identifier> rest;

    void print(std::ostream &out) const override;

    MatchMapping(): Pattern(NodeKind::MatchMapping) {}
};

struct MatchClass: public Pattern {
    ExprNode *        cls;
    Array<Pattern *>  patterns;
    Array<Identifier> kwd_attrs;
    Array<Pattern *>  kwd_patterns;

    void print(std::ostream &out) const override;

    MatchClass(): Pattern(NodeKind::MatchClass) {}
};

struct MatchStar: public Pattern {
    Optional<Identifier> name;

    void print(std::ostream &out) const override;

    MatchStar(): Pattern(NodeKind::MatchStar) {}
};

struct MatchAs: public Pattern {
    Optional<Pattern *>  pattern;
    Optional<Identifier> name;

    void print(std::ostream &out) const override;

    MatchAs(): Pattern(NodeKind::MatchAs) {}
};

struct MatchOr: public Pattern {
    Array<Pattern *> patterns;

    void print(std::ostream &out) const override;

    MatchOr(): Pattern(NodeKind::MatchOr) {}
};

struct MatchCase {
    Pattern *            pattern;
    Optional<ExprNode *> guard;
    Array<StmtNode *>    body;

    void print(std::ostream &out, int indent) const;
};

// Expressions
// -----------

struct BoolOp: public ExprNode {
    BoolOperator      op;
    Array<ExprNode *> values;

    void print(std::ostream &out, int indent) const;

    BoolOp(): ExprNode(NodeKind::BoolOp) {}
};

struct NamedExpr: public ExprNode {
    ExprNode *target = nullptr;
    ExprNode *value  = nullptr;

    NamedExpr(): ExprNode(NodeKind::NamedExpr) {}

    void print(std::ostream &out, int indent) const;
};

struct BinOp: public ExprNode {
    ExprNode *     left = nullptr;
    BinaryOperator op;
    ExprNode *     right = nullptr;

    void print(std::ostream &out, int indent) const;

    BinOp(): ExprNode(NodeKind::BinOp) {}
};

struct UnaryOp: public ExprNode {
    UnaryOperator op;
    ExprNode *    operand;

    void print(std::ostream &out, int indent) const;

    UnaryOp(): ExprNode(NodeKind::UnaryOp) {}
};

struct Lambda: public ExprNode {
    Arguments args;
    ExprNode *body = nullptr;

    void print(std::ostream &out, int indent) const;

    Lambda(): ExprNode(NodeKind::Lambda) {}
};

struct IfExp: public ExprNode {
    ExprNode *test   = nullptr;
    ExprNode *body   = nullptr;
    ExprNode *orelse = nullptr;

    void print(std::ostream &out, int indent) const;

    IfExp(): ExprNode(NodeKind::IfExp) {}
};

struct DictExpr: public ExprNode {
    Array<ExprNode *> keys;
    Array<ExprNode *> values;

    void print(std::ostream &out, int indent) const;

    DictExpr(): ExprNode(NodeKind::DictExpr) {}
};

struct SetExpr: public ExprNode {
    Array<ExprNode *> elts;

    void print(std::ostream &out, int indent) const;

    SetExpr(): ExprNode(NodeKind::SetExpr) {}
};

struct ListComp: public ExprNode {
    ExprNode *           elt = nullptr;
    Array<Comprehension> generators;

    void print(std::ostream &out, int indent) const;

    ListComp(): ExprNode(NodeKind::ListComp) {}
};

struct GeneratorExp: public ExprNode {
    ExprNode *           elt = nullptr;
    Array<Comprehension> generators;

    void print(std::ostream &out, int indent) const;

    GeneratorExp(): ExprNode(NodeKind::GeneratorExp) {}
};

struct SetComp: public ExprNode {
    ExprNode *           elt = nullptr;
    Array<Comprehension> generators;

    void print(std::ostream &out, int indent) const;

    SetComp(): ExprNode(NodeKind::SetComp) {}
};

struct DictComp: public ExprNode {
    ExprNode *           key   = nullptr;
    ExprNode *           value = nullptr;
    Array<Comprehension> generators;

    void print(std::ostream &out, int indent) const;

    DictComp(): ExprNode(NodeKind::DictComp) {}
};

// the grammar constrains where yield expressions can occur
struct Await: public ExprNode {
    ExprNode *value;

    void print(std::ostream &out, int indent) const;

    Await(): ExprNode(NodeKind::Await) {}
};

struct Yield: public ExprNode {
    Optional<ExprNode *> value;

    void print(std::ostream &out, int indent) const;

    Yield(): ExprNode(NodeKind::Yield) {}
};

struct YieldFrom: public ExprNode {
    ExprNode *value = nullptr;

    void print(std::ostream &out, int indent) const;

    YieldFrom(): ExprNode(NodeKind::YieldFrom) {}
};

// need sequences for compare to distinguish between
// x < 4 < 3 and (x < 4) < 3
struct Compare: public ExprNode {
    ExprNode *         left = nullptr;
    Array<CmpOperator> ops;
    Array<ExprNode *>  comparators;

    void print(std::ostream &out, int indent) const;

    Compare(): ExprNode(NodeKind::Compare) {}
};

struct Call: public ExprNode {
    ExprNode *        func = nullptr;
    Array<ExprNode *> args;
    Array<Keyword>    keywords;

    void print(std::ostream &out, int indent) const override;

    Call(): ExprNode(NodeKind::Call) {}
};

struct JoinedStr: public ExprNode {
    Array<ExprNode *> values;

    JoinedStr(): ExprNode(NodeKind::JoinedStr) {}

    void print(std::ostream &out, int indent) const override;
};

struct FormattedValue: public ExprNode {
    ExprNode *               value      = nullptr;
    Optional<ConversionKind> conversion = ConversionKind::None;
    // defined as ExprNode*
    JoinedStr format_spec;

    FormattedValue(): ExprNode(NodeKind::FormattedValue) {}

    void print(std::ostream &out, int indent) const override;
};

struct Constant: public ExprNode {
    ConstantValue    value;
    Optional<String> kind;

    void print(std::ostream &out, int indent) const override;

    template <typename T>
    Constant(T const &v): ExprNode(NodeKind::Constant), value(v) {}

    Constant(): Constant(ConstantValue::invalid_t()) {}
};

// the following expression can appear in assignment context
struct Attribute: public ExprNode {
    ExprNode *  value = nullptr;
    Identifier  attr;
    ExprContext ctx;

    void print(std::ostream &out, int indent) const override {
        value->print(out, indent);
        out << ".";
        out << attr;
    }

    Attribute(): ExprNode(NodeKind::Attribute) {}
};

struct Subscript: public ExprNode {
    ExprNode *  value = nullptr;
    ExprNode *  slice = nullptr;
    ExprContext ctx;

    void print(std::ostream &out, int indent) const override {
        value->print(out, indent);
        out << "[";
        slice->print(out, indent);
        out << "]";
    }

    Subscript(): ExprNode(NodeKind::Subscript) {}
};

struct Starred: public ExprNode {
    ExprNode *  value = nullptr;
    ExprContext ctx;

    void print(std::ostream &out, int indent) const override {
        out << "*";
        value->print(out, indent);
    }

    Starred(): ExprNode(NodeKind::Starred) {}
};

struct Name: public ExprNode {
    Identifier  id;
    ExprContext ctx;

    // SEMA
    int varid = -1;

    void print(std::ostream &out, int indent) const override;

    Name(): ExprNode(NodeKind::Name) {}
};

struct ListExpr: public ExprNode {
    Array<ExprNode *> elts;
    ExprContext       ctx;

    void print(std::ostream &out, int indent) const;

    ListExpr(): ExprNode(NodeKind::ListExpr) {}
};

struct TupleExpr: public ExprNode {
    Array<ExprNode *> elts;
    ExprContext       ctx;

    void print(std::ostream &out, int indent) const;

    TupleExpr(): ExprNode(NodeKind::TupleExpr) {}
};

// can appear only in Subscript
struct Slice: public ExprNode {
    Optional<ExprNode *> lower;
    Optional<ExprNode *> upper;
    Optional<ExprNode *> step;

    void print(std::ostream &out, int indent) const;

    Slice(): ExprNode(NodeKind::Slice) {}
};

// Modules
// -------
struct Module: public ModNode {
    Array<StmtNode *> body;

    Optional<String> docstring;

    Module(): ModNode(NodeKind::Module) {}

    void print(std::ostream &out, int) const;
};

struct Interactive: public ModNode {
    Array<StmtNode *> body;

    Interactive(): ModNode(NodeKind::Interactive) {}
};

struct Expression: public ModNode {
    ExprNode *body = nullptr;

    Expression(): ModNode(NodeKind::Expression) {}
};

struct FunctionType: public ModNode {
    Array<ExprNode *> argtypes;
    ExprNode *        returns = nullptr;

    FunctionType(): ModNode(NodeKind::FunctionType) {}
};

// Statements
// ----------
struct Inline: public StmtNode {
    // <stmt>; <stmt>
    Array<StmtNode *> body;

    void print(std::ostream &out, int indent) const override;

    Inline(): StmtNode(NodeKind::Inline) {}
};

struct FunctionDef: public StmtNode {
    Identifier           name;
    Arguments            args;
    Array<StmtNode *>    body;
    Array<ExprNode *>    decorator_list = {};
    Optional<ExprNode *> returns;
    String               type_comment;

    Optional<String> docstring;
    bool             async : 1 = false;

    void print(std::ostream &out, int indent) const override;

    FunctionDef(): StmtNode(NodeKind::FunctionDef) {}
};

struct AsyncFunctionDef: public FunctionDef {};

struct ClassDef: public StmtNode {
    Identifier        name;
    Array<ExprNode *> bases;
    Array<Keyword>    keywords;
    Array<StmtNode *> body;
    Array<ExprNode *> decorator_list = {};

    Optional<String> docstring;

    void print(std::ostream &out, int indent) const override;

    ClassDef(): StmtNode(NodeKind::ClassDef) {}

    // To match python AST the body of the class is a simple Array of statement
    // but this is not very convenient for semantic analysis
    //

    struct Attr {
        StringRef name;
        int       offset = -1;
        StmtNode *stmt   = nullptr;
        ExprNode *type   = nullptr;

        operator bool() { return name != StringRef(); }
    };
    Dict<StringRef, Attr> attributes;

    Attr &get_attribute(StringRef name) { return attributes[name]; }

    bool insert_attribute(StringRef name, StmtNode *stmt, ExprNode *type = nullptr) {
        auto v = attributes[name];

        if (v.name == StringRef()) {
            attributes[name] = Attr{name, int(attributes.size()), stmt, type};
            return true;
        }

        if (!v.type && type) {
            v.type = type;
        }
        return false;
    }
};

struct Return: public StmtNode {
    Optional<ExprNode *> value;

    void print(std::ostream &out, int indent) const;

    Return(): StmtNode(NodeKind::Return) {}
};

struct Delete: public StmtNode {
    Array<ExprNode *> targets;

    void print(std::ostream &out, int indent) const;

    Delete(): StmtNode(NodeKind::Delete) {}
};

struct Assign: public StmtNode {
    // The array is useless;
    // only the first element is ever populated ?
    // a, b = c
    // Tuple(a, b) = c
    //
    Array<ExprNode *> targets;
    ExprNode *        value = nullptr;
    Optional<String>  type_comment;

    void print(std::ostream &out, int indent) const;

    Assign(): StmtNode(NodeKind::Assign) {}
};

struct AugAssign: public StmtNode {
    ExprNode *     target = nullptr;
    BinaryOperator op;
    ExprNode *     value = nullptr;

    void print(std::ostream &out, int indent) const;

    AugAssign(): StmtNode(NodeKind::AugAssign) {}
};

// 'simple' indicates that we annotate simple name without parens
struct AnnAssign: public StmtNode {
    ExprNode *           target     = nullptr;
    ExprNode *           annotation = nullptr;
    Optional<ExprNode *> value;
    int                  simple;

    void print(std::ostream &out, int indent) const;

    AnnAssign(): StmtNode(NodeKind::AnnAssign) {}
};

// use 'orelse' because else is a keyword in target languages
struct For: public StmtNode {
    ExprNode *        target = nullptr;
    ExprNode *        iter   = nullptr;
    Array<StmtNode *> body;
    Array<StmtNode *> orelse;
    Optional<String>  type_comment;

    bool async = false;

    For(): StmtNode(NodeKind::For) {}

    void print(std::ostream &out, int indent) const;
};

// Keeping it for consistency with python docs, but useless
struct AsyncFor: public For {};

struct While: public StmtNode {
    ExprNode *        test = nullptr;
    Array<StmtNode *> body;
    Array<StmtNode *> orelse;

    While(): StmtNode(NodeKind::While) {}

    void print(std::ostream &out, int indent) const;
};

struct If: public StmtNode {
    ExprNode *        test = nullptr;
    Array<StmtNode *> body;
    Array<StmtNode *> orelse;

    // alternative representation that diverges from
    // the python ast
    Array<ExprNode *>        tests;
    Array<Array<StmtNode *>> bodies;

    void print(std::ostream &out, int indent) const;

    If(): StmtNode(NodeKind::If) {}
};

struct With: public StmtNode {
    Array<WithItem>   items;
    Array<StmtNode *> body;
    Optional<String>  type_comment;

    bool async = false;

    void print(std::ostream &out, int indent) const;

    With(): StmtNode(NodeKind::With) {}
};

// Keeping it for consistency with python docs, but useless
struct AsyncWith: public With {};

struct Raise: public StmtNode {
    Optional<ExprNode *> exc;
    Optional<ExprNode *> cause;

    void print(std::ostream &out, int indent) const;

    Raise(): StmtNode(NodeKind::Raise) {}
};

struct Try: public StmtNode {
    Array<StmtNode *>    body;
    Array<ExceptHandler> handlers;
    Array<StmtNode *>    orelse;
    Array<StmtNode *>    finalbody;

    void print(std::ostream &out, int indent) const;

    Try(): StmtNode(NodeKind::Try) {}
};

struct Assert: public StmtNode {
    ExprNode *           test = nullptr;
    Optional<ExprNode *> msg;

    void print(std::ostream &out, int indent) const;

    Assert(): StmtNode(NodeKind::Assert) {}
};

struct Import: public StmtNode {
    Array<Alias> names;

    void print(std::ostream &out, int indent) const;

    Import(): StmtNode(NodeKind::Import) {}
};

struct ImportFrom: public StmtNode {
    Optional<Identifier> module;
    Array<Alias>         names;
    Optional<int>        level;

    void print(std::ostream &out, int indent) const;

    ImportFrom(): StmtNode(NodeKind::ImportFrom) {}
};

struct Global: public StmtNode {
    Array<Identifier> names;

    void print(std::ostream &out, int indent) const;

    Global(): StmtNode(NodeKind::Global) {}
};

struct Nonlocal: public StmtNode {
    Array<Identifier> names;

    void print(std::ostream &out, int indent) const;

    Nonlocal(): StmtNode(NodeKind::Nonlocal) {}
};

struct Expr: public StmtNode {
    ExprNode *value = nullptr;

    void print(std::ostream &out, int indent) const;

    Expr(): StmtNode(NodeKind::Expr) {}
};

struct Pass: public StmtNode {
    void print(std::ostream &out, int indent) const;

    Pass(): StmtNode(NodeKind::Pass) {}
};

struct Break: public StmtNode {
    void print(std::ostream &out, int indent) const;

    Break(): StmtNode(NodeKind::Break) {}
};

struct Continue: public StmtNode {
    void print(std::ostream &out, int indent) const;

    Continue(): StmtNode(NodeKind::Continue) {}
};

struct Match: public StmtNode {
    ExprNode *       subject;
    Array<MatchCase> cases;

    void print(std::ostream &out, int indent) const;

    Match(): StmtNode(NodeKind::Match) {}
};

//
struct NotImplementedStmt: public StmtNode {
    NotImplementedStmt(): StmtNode(NodeKind::Invalid) {}

    void print(std::ostream &out, int indent) const { out << "<not implemented>"; }
};

struct NotImplementedExpr: public ExprNode {
    NotImplementedExpr(): ExprNode(NodeKind::Invalid) {}

    void print(std::ostream &out, int indent) const { out << "<not implemented>"; }
};

struct NotAllowedEpxr: public ExprNode {
    NotAllowedEpxr(): ExprNode(NodeKind::Invalid) {}

    String msg;

    void print(std::ostream &out, int indent) const { out << "<not allowed: " << msg << ">"; }
};

struct Arrow: public ExprNode {
    Arrow(): ExprNode(NodeKind::Arrow) {}

    Array<ExprNode *> args;
    ExprNode *        returns = nullptr;

    void print(std::ostream &out, int indent) const override;
};

struct DictType: public ExprNode {
    DictType(): ExprNode(NodeKind::DictType) {}

    ExprNode *key   = nullptr;
    ExprNode *value = nullptr;

    void print(std::ostream &out, int indent) const override;
};

struct SetType: public ExprNode {
    SetType(): ExprNode(NodeKind::SetType) {}

    ExprNode *value = nullptr;

    void print(std::ostream &out, int indent) const override;
};

struct ArrayType: public ExprNode {
    ArrayType(): ExprNode(NodeKind::ArrayType) {}

    ExprNode *value = nullptr;

    void print(std::ostream &out, int indent) const override;
};

struct TupleType: public ExprNode {
    TupleType(): ExprNode(NodeKind::TupleType) {}

    Array<ExprNode *> types;

    void print(std::ostream &out, int indent) const override;
};

struct BuiltinType: public ExprNode {
    BuiltinType(): ExprNode(NodeKind::BuiltinType) {}
    StringRef name;

    void print(std::ostream &out, int indent) const override;
};

// we need that to convert ClassDef which is a statement
// into an expression
//
// Actually: I can use a Name for that
//
struct ClassType: public ExprNode {
    ClassType(): ExprNode(NodeKind::ClassType) {}
    ClassDef *def;

    void print(std::ostream &out, int indent) const override;
};

// This is essentially compile time lookup
// no-need for the function to actually exist at runtime
#define SPECGEN(name)                   \
    template <>                         \
    struct NodeTrait<name> {            \
        enum Constants                  \
        { kind = int(NodeKind::name) }; \
    };

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, _)  SPECGEN(name)
#define STMT(name, _)  SPECGEN(name)
#define MOD(name, _)   SPECGEN(name)
#define MATCH(name, _) SPECGEN(name)

NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef SPECGEN

// Safe cast
template <typename T>
T *cast(Node *obj) {
    if (obj->is_instance<T>()) {
        return (T *)obj;
    }
    return nullptr;
}

template <typename T>
T *checked_cast(Node *obj) {
    assert(obj->is_instance<T>(),
           fmt::format("Cast type is not compatible {} != {}", str(obj->kind), str(nodekind<T>())));
    return cast<T>(obj);
}

} // namespace lython
#endif
