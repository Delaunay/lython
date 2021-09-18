#ifndef LYTHON_SEXPR_HEADER
#define LYTHON_SEXPR_HEADER

#include <memory>

#include "../dtypes.h"
#include "../utilities/optional.h"

#include "constant.h"
#include "object.h"

namespace lython {

using Identifier = String;

enum class NodeKind : int8_t
{
    Invalid,

    EXPR_START,

    BoolOp,
    NamedExpr,
    BinOp,
    UnaryOp,
    Lambda,
    IfExp,
    DictExpr,
    SetExpr,
    ListComp,
    GeneratorExp,
    SetComp,
    DictComp,
    Await,
    Yield,
    YieldFrom,
    Compare,
    Call,
    JoinedStr,
    FormattedValue,
    Constant,
    Attribute,
    Subscript,
    Starred,
    Name,
    ListExpr,
    TupleExpr,
    Slice,

    EXPR_END,
    MODULE_START = EXPR_END,

    Module,
    Interactive,
    Expression,
    FunctionType,

    MODULE_END,
    STMT_START = MODULE_END,

    FunctionDef,
    ClassDef,
    Return,
    Delete,
    Assign,
    AugAssign,
    AnnAssign,
    For,
    While,
    If,
    With,
    Raise,
    Try,
    Assert,
    Import,
    ImportFrom,
    Global,
    Nonlocal,
    Expr,
    Pass,
    Break,
    Continue,
    Match,

    STMT_END,
    PAT_START = STMT_END,

    MatchValue,
    MatchSingleton,
    MatchSequence,
    MatchMapping,
    MatchClass,
    MatchStar,
    MatchAs,
    MatchOr,
    MatchCase,

    PAT_END
};

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

struct Node: public GCObject {
    // I think only statements need the indentaion
    virtual void print(std::ostream &out, int indent = 0) const {}

    String __str__() const;

    Node(NodeKind _kind): kind(_kind) {}

    virtual NodeFamily family() const = 0;

    const NodeKind kind;
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
    FloorDiv
};

enum class BoolOperator : int8_t
{
    And,
    Or
};

enum class UnaryOperator : int8_t
{
    Invert,
    Not,
    UAdd,
    USub,
};

enum class ExprContext : int8_t
{
    Load,
    Store,
    Del
};

enum class CmpOperator : int8_t
{
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
    Optional<Identifier> arg; // why is this optional ?
    ExprNode *           value = nullptr;

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

    virtual void print(std::ostream &out) const {}

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

    void print(std::ostream &out, int indent) const override;

    Constant(): ExprNode(NodeKind::Constant) {}
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

    void print(std::ostream &out, int indent) const override { out << id; }

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

    String docstring;

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
struct FunctionDef: public StmtNode {
    Identifier           name;
    Arguments            args;
    Array<StmtNode *>    body;
    Array<ExprNode *>    decorator_list;
    Optional<ExprNode *> returns;
    String               type_comment;

    String docstring;
    bool   async : 1 = false;

    void print(std::ostream &out, int indent) const override;

    FunctionDef(): StmtNode(NodeKind::FunctionDef) {}
};

struct AsyncFunctionDef: public FunctionDef {};

struct ClassDef: public StmtNode {
    Identifier        name;
    Array<ExprNode *> bases;
    Array<Keyword>    keywords;
    Array<StmtNode *> body;
    Array<ExprNode *> decorator_list;

    String docstring;

    void print(std::ostream &out, int indent) const override;

    ClassDef(): StmtNode(NodeKind::ClassDef) {}
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

} // namespace lython
#endif
