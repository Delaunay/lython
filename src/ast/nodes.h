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

enum class BoolOperator : int8_t
{
    None,
    And,
    Or
};

enum class UnaryOperator : int8_t
{
    None,
    Invert, // ~
    Not,    // !
    UAdd,   // +
    USub,   // -
};

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

struct Comprehension {
    ExprNode *        target = nullptr;
    ExprNode *        iter   = nullptr;
    Array<ExprNode *> ifs;
    int               is_async : 1;
};

struct ExceptHandler: public CommonAttributes {
    Optional<ExprNode *> type;
    Optional<Identifier> name;
    Array<StmtNode *>    body;
};

struct Arg: public CommonAttributes {
    Identifier           arg;
    Optional<ExprNode *> annotation;
    Optional<String>     type_comment;
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
};

struct Keyword: public CommonAttributes {
    Identifier arg; // why is this optional ?
                    // it is marked as optional in the python AST
    ExprNode *value = nullptr;
};

struct Alias {
    Identifier           name;
    Optional<Identifier> asname;
};

struct WithItem {
    ExprNode *           context_expr = nullptr;
    Optional<ExprNode *> optional_vars;
};

struct TypeIgnore {
    int    lineno;
    String tag;
};

struct Pattern: public CommonAttributes, public Node {
    Pattern(NodeKind kind): Node(kind) {}

    NodeFamily family() const override { return NodeFamily::Pattern; }
};

struct MatchValue: public Pattern {
    ExprNode *value;

    MatchValue(): Pattern(NodeKind::MatchValue) {}
};

struct MatchSingleton: public Pattern {
    ConstantValue value;

    MatchSingleton(): Pattern(NodeKind::MatchSingleton) {}
};

struct MatchSequence: public Pattern {
    Array<Pattern *> patterns;

    MatchSequence(): Pattern(NodeKind::MatchSequence) {}
};

// The optional "rest" MatchMapping parameter handles capturing extra mapping keys
struct MatchMapping: public Pattern {
    Array<ExprNode *>    keys;
    Array<Pattern *>     patterns;
    Optional<Identifier> rest;

    MatchMapping(): Pattern(NodeKind::MatchMapping) {}
};

struct MatchClass: public Pattern {
    ExprNode *        cls;
    Array<Pattern *>  patterns;
    Array<Identifier> kwd_attrs;
    Array<Pattern *>  kwd_patterns;

    MatchClass(): Pattern(NodeKind::MatchClass) {}
};

struct MatchStar: public Pattern {
    Optional<Identifier> name;

    MatchStar(): Pattern(NodeKind::MatchStar) {}
};

struct MatchAs: public Pattern {
    Optional<Pattern *>  pattern;
    Optional<Identifier> name;

    MatchAs(): Pattern(NodeKind::MatchAs) {}
};

struct MatchOr: public Pattern {
    Array<Pattern *> patterns;

    MatchOr(): Pattern(NodeKind::MatchOr) {}
};

struct MatchCase {
    Pattern *            pattern;
    Optional<ExprNode *> guard;
    Array<StmtNode *>    body;
};

// Expressions
// -----------

struct BoolOp: public ExprNode {
    BoolOperator      op;
    Array<ExprNode *> values;

    BoolOp(): ExprNode(NodeKind::BoolOp) {}
};

struct NamedExpr: public ExprNode {
    ExprNode *target = nullptr;
    ExprNode *value  = nullptr;

    NamedExpr(): ExprNode(NodeKind::NamedExpr) {}
};

struct BinOp: public ExprNode {
    ExprNode *     left = nullptr;
    BinaryOperator op;
    ExprNode *     right = nullptr;

    BinOp(): ExprNode(NodeKind::BinOp) {}
};

struct UnaryOp: public ExprNode {
    UnaryOperator op;
    ExprNode *    operand;

    UnaryOp(): ExprNode(NodeKind::UnaryOp) {}
};

struct Lambda: public ExprNode {
    Arguments args;
    ExprNode *body = nullptr;

    Lambda(): ExprNode(NodeKind::Lambda) {}
};

struct IfExp: public ExprNode {
    ExprNode *test   = nullptr;
    ExprNode *body   = nullptr;
    ExprNode *orelse = nullptr;

    IfExp(): ExprNode(NodeKind::IfExp) {}
};

struct DictExpr: public ExprNode {
    Array<ExprNode *> keys;
    Array<ExprNode *> values;

    DictExpr(): ExprNode(NodeKind::DictExpr) {}
};

struct SetExpr: public ExprNode {
    Array<ExprNode *> elts;

    SetExpr(): ExprNode(NodeKind::SetExpr) {}
};

struct ListComp: public ExprNode {
    ExprNode *           elt = nullptr;
    Array<Comprehension> generators;

    ListComp(): ExprNode(NodeKind::ListComp) {}
};

struct GeneratorExp: public ExprNode {
    ExprNode *           elt = nullptr;
    Array<Comprehension> generators;

    GeneratorExp(): ExprNode(NodeKind::GeneratorExp) {}
};

struct SetComp: public ExprNode {
    ExprNode *           elt = nullptr;
    Array<Comprehension> generators;

    SetComp(): ExprNode(NodeKind::SetComp) {}
};

struct DictComp: public ExprNode {
    ExprNode *           key   = nullptr;
    ExprNode *           value = nullptr;
    Array<Comprehension> generators;

    DictComp(): ExprNode(NodeKind::DictComp) {}
};

// the grammar constrains where yield expressions can occur
struct Await: public ExprNode {
    ExprNode *value;

    Await(): ExprNode(NodeKind::Await) {}
};

#undef Yield
struct Yield: public ExprNode {
    Optional<ExprNode *> value;

    Yield(): ExprNode(NodeKind::Yield) {}
};

struct YieldFrom: public ExprNode {
    ExprNode *value = nullptr;

    YieldFrom(): ExprNode(NodeKind::YieldFrom) {}
};

// need sequences for compare to distinguish between
// x < 4 < 3 and (x < 4) < 3
struct Compare: public ExprNode {
    ExprNode *         left = nullptr;
    Array<CmpOperator> ops;
    Array<ExprNode *>  comparators;

    Compare(): ExprNode(NodeKind::Compare) {}
};

struct Call: public ExprNode {
    ExprNode *        func = nullptr;
    Array<ExprNode *> args;
    Array<Keyword>    keywords;

    Call(): ExprNode(NodeKind::Call) {}
};

struct JoinedStr: public ExprNode {
    Array<ExprNode *> values;

    JoinedStr(): ExprNode(NodeKind::JoinedStr) {}
};

struct FormattedValue: public ExprNode {
    ExprNode *               value      = nullptr;
    Optional<ConversionKind> conversion = ConversionKind::None;
    // defined as ExprNode*
    JoinedStr format_spec;

    FormattedValue(): ExprNode(NodeKind::FormattedValue) {}
};

struct Constant: public ExprNode {
    ConstantValue    value;
    Optional<String> kind;

    template <typename T>
    Constant(T const &v): ExprNode(NodeKind::Constant), value(v) {}

    Constant(): Constant(ConstantValue::invalid_t()) {}
};

// the following expression can appear in assignment context
struct Attribute: public ExprNode {
    ExprNode *  value = nullptr;
    Identifier  attr;
    ExprContext ctx;

    Attribute(): ExprNode(NodeKind::Attribute) {}
};

struct Subscript: public ExprNode {
    ExprNode *  value = nullptr;
    ExprNode *  slice = nullptr;
    ExprContext ctx;

    Subscript(): ExprNode(NodeKind::Subscript) {}
};

struct Starred: public ExprNode {
    ExprNode *  value = nullptr;
    ExprContext ctx;

    Starred(): ExprNode(NodeKind::Starred) {}
};

struct Name: public ExprNode {
    Identifier  id;
    ExprContext ctx;

    // SEMA
    int varid = -1;

    Name(): ExprNode(NodeKind::Name) {}
};

struct ListExpr: public ExprNode {
    Array<ExprNode *> elts;
    ExprContext       ctx;

    ListExpr(): ExprNode(NodeKind::ListExpr) {}
};

struct TupleExpr: public ExprNode {
    Array<ExprNode *> elts;
    ExprContext       ctx;

    TupleExpr(): ExprNode(NodeKind::TupleExpr) {}
};

// can appear only in Subscript
struct Slice: public ExprNode {
    Optional<ExprNode *> lower;
    Optional<ExprNode *> upper;
    Optional<ExprNode *> step;

    Slice(): ExprNode(NodeKind::Slice) {}
};

// Modules
// -------
struct Module: public ModNode {
    Array<StmtNode *> body;

    Optional<String> docstring;

    Module(): ModNode(NodeKind::Module) {}
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

    Return(): StmtNode(NodeKind::Return) {}
};

struct Delete: public StmtNode {
    Array<ExprNode *> targets;

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

    Assign(): StmtNode(NodeKind::Assign) {}
};

struct AugAssign: public StmtNode {
    ExprNode *     target = nullptr;
    BinaryOperator op;
    ExprNode *     value = nullptr;

    AugAssign(): StmtNode(NodeKind::AugAssign) {}
};

// 'simple' indicates that we annotate simple name without parens
struct AnnAssign: public StmtNode {
    ExprNode *           target     = nullptr;
    ExprNode *           annotation = nullptr;
    Optional<ExprNode *> value;
    int                  simple;

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
};

// Keeping it for consistency with python docs, but useless
struct AsyncFor: public For {};

struct While: public StmtNode {
    ExprNode *        test = nullptr;
    Array<StmtNode *> body;
    Array<StmtNode *> orelse;

    While(): StmtNode(NodeKind::While) {}
};

struct If: public StmtNode {
    ExprNode *        test = nullptr;
    Array<StmtNode *> body;
    Array<StmtNode *> orelse;

    // alternative representation that diverges from
    // the python ast
    Array<ExprNode *>        tests;
    Array<Array<StmtNode *>> bodies;

    If(): StmtNode(NodeKind::If) {}
};

struct With: public StmtNode {
    Array<WithItem>   items;
    Array<StmtNode *> body;
    Optional<String>  type_comment;

    bool async = false;

    With(): StmtNode(NodeKind::With) {}
};

// Keeping it for consistency with python docs, but useless
struct AsyncWith: public With {};

struct Raise: public StmtNode {
    Optional<ExprNode *> exc;
    Optional<ExprNode *> cause;

    Raise(): StmtNode(NodeKind::Raise) {}
};

struct Try: public StmtNode {
    Array<StmtNode *>    body;
    Array<ExceptHandler> handlers;
    Array<StmtNode *>    orelse;
    Array<StmtNode *>    finalbody;

    Try(): StmtNode(NodeKind::Try) {}
};

struct Assert: public StmtNode {
    ExprNode *           test = nullptr;
    Optional<ExprNode *> msg;

    Assert(): StmtNode(NodeKind::Assert) {}
};

struct Import: public StmtNode {
    Array<Alias> names;

    Import(): StmtNode(NodeKind::Import) {}
};

struct ImportFrom: public StmtNode {
    Optional<Identifier> module;
    Array<Alias>         names;
    Optional<int>        level;

    ImportFrom(): StmtNode(NodeKind::ImportFrom) {}
};

struct Global: public StmtNode {
    Array<Identifier> names;

    Global(): StmtNode(NodeKind::Global) {}
};

struct Nonlocal: public StmtNode {
    Array<Identifier> names;

    Nonlocal(): StmtNode(NodeKind::Nonlocal) {}
};

struct Expr: public StmtNode {
    ExprNode *value = nullptr;

    Expr(): StmtNode(NodeKind::Expr) {}
};

struct Pass: public StmtNode {

    Pass(): StmtNode(NodeKind::Pass) {}
};

struct Break: public StmtNode {

    Break(): StmtNode(NodeKind::Break) {}
};

struct Continue: public StmtNode {

    Continue(): StmtNode(NodeKind::Continue) {}
};

struct Match: public StmtNode {
    ExprNode *       subject;
    Array<MatchCase> cases;

    Match(): StmtNode(NodeKind::Match) {}
};

//
struct NotImplementedStmt: public StmtNode {
    NotImplementedStmt(): StmtNode(NodeKind::Invalid) {}
};

struct NotImplementedExpr: public ExprNode {
    NotImplementedExpr(): ExprNode(NodeKind::Invalid) {}
};

struct NotAllowedEpxr: public ExprNode {
    NotAllowedEpxr(): ExprNode(NodeKind::Invalid) {}

    String msg;
};

struct Arrow: public ExprNode {
    Arrow(): ExprNode(NodeKind::Arrow) {}

    Array<ExprNode *> args;
    ExprNode *        returns = nullptr;
};

struct DictType: public ExprNode {
    DictType(): ExprNode(NodeKind::DictType) {}

    ExprNode *key   = nullptr;
    ExprNode *value = nullptr;
};

struct SetType: public ExprNode {
    SetType(): ExprNode(NodeKind::SetType) {}

    ExprNode *value = nullptr;
};

struct ArrayType: public ExprNode {
    ArrayType(): ExprNode(NodeKind::ArrayType) {}

    ExprNode *value = nullptr;
};

struct TupleType: public ExprNode {
    TupleType(): ExprNode(NodeKind::TupleType) {}

    Array<ExprNode *> types;
};

struct BuiltinType: public ExprNode {
    BuiltinType(): ExprNode(NodeKind::BuiltinType) {}
    StringRef name;
};

// we need that to convert ClassDef which is a statement
// into an expression
//
// Actually: I can use a Name for that
//
struct ClassType: public ExprNode {
    ClassType(): ExprNode(NodeKind::ClassType) {}
    ClassDef *def;
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
    if (obj == nullptr) {
        return nullptr;
    }
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
