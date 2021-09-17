#ifndef LYTHON_SEXPR_HEADER
#define LYTHON_SEXPR_HEADER

#include <memory>

#include "../dtypes.h"
#include "../utilities/optional.h"

#include "constant.h"
#include "object.h"

namespace lython {

using Identifier = String;

// col_offset is the byte offset in the utf8 string the parser uses
struct CommonAttributes {
    int lineno;
    int col_offset;
    Optional<int> end_lineno;
    Optional<int> end_col_offset;
};

struct Node {
    // I think only statements need the indentaion
	virtual void print(std::ostream& out, int indent = 0) const {}

    String __str__() const;
};

struct ModNode: public GCObject, public Node {
    ModNode():
        GCObject(ObjectKind::Module)
    {}
};

struct StmtNode: public CommonAttributes, public GCObject, public Node {
    StmtNode():
        GCObject(ObjectKind::Statement)
    {}
};

struct ExprNode: public CommonAttributes, public GCObject, public Node {
    ExprNode():
        GCObject(ObjectKind::Expression)
    {}
};

enum class ConversionKind {
    None = -1,
    String = 115,
    Representation = 114,
    ASCII = 97
};

enum class BinaryOperator  {
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

enum class BoolOperator {
    And,
    Or
};

enum class UnaryOperator {
    Invert,
    Not,
    UAdd,
    USub,
};

enum class ExprContext {
    Load,
    Store,
    Del
};

enum class CmpOperator {
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
    ExprNode* target = nullptr;
    ExprNode* iter = nullptr;
    Array<ExprNode*> ifs;
    int is_async;

    String __str__() const;

    void print(std::ostream& out, int indent) const;
};


void print(std::ostream& out, int indent, Array<StmtNode*> const& body);

struct ExceptHandler: public CommonAttributes {
    Optional<ExprNode*> type;
    Optional<Identifier> name;
    Array<StmtNode*> body;

    void print(std::ostream& out, int indent) const;
};

struct Arg: public CommonAttributes {
    Identifier arg;
    Optional<ExprNode*> annotation;
    Optional<String> type_comment;

    void print(std::ostream& out, int indent = 0) const;
};

struct Arguments {
    Arguments() = default;

    Arguments(Arguments const&) = default;

    Array<Arg> posonlyargs;
    Array<Arg> args;
	Optional<Arg> vararg;				// *args
    Array<Arg> kwonlyargs;
    Array<ExprNode*> kw_defaults;
    Optional<Arg> kwarg;				// **kwargs
    Array<ExprNode*> defaults;

    void print(std::ostream& out, int indent) const;
};

struct Keyword: public CommonAttributes {
    Optional<Identifier> arg;   // why is this optional ?
    ExprNode* value = nullptr;

    void print(std::ostream& out, int indent) const;
};

struct Alias {
    Identifier name;
    Optional<Identifier> asname;

    void print(std::ostream& out, int indent) const;
};

struct WithItem {
    ExprNode* context_expr = nullptr;
    Optional<ExprNode*> optional_vars;

    void print(std::ostream& out, int indent) const;
};

struct TypeIgnore {
    int lineno;
    String tag;
};

struct Pattern: public CommonAttributes, public GCObject {
    Pattern():
        GCObject(ObjectKind::Pattern)
    {}

    virtual void print(std::ostream& out) const {}

    String __str__() const;
};

struct MatchValue : public Pattern {
    ExprNode* value;

    void print(std::ostream& out) const override;
};

struct MatchSingleton: public Pattern {
    ConstantValue value;

    void print(std::ostream& out) const override;
};

struct MatchSequence: public Pattern {
    Array<Pattern*> patterns;

    void print(std::ostream& out) const override;
};

// The optional "rest" MatchMapping parameter handles capturing extra mapping keys
struct MatchMapping: public Pattern {
    Array<ExprNode*> keys;
    Array<Pattern*> patterns;
    Optional<Identifier> rest;

    void print(std::ostream& out) const override;
};

struct MatchClass: public Pattern {
    ExprNode* cls;
    Array<Pattern*> patterns;
    Array<Identifier> kwd_attrs;
    Array<Pattern*> kwd_patterns;

    void print(std::ostream& out) const override;
};

struct MatchStar: public Pattern {
    Optional<Identifier> name;

    void print(std::ostream& out) const override;
};

struct MatchAs: public Pattern {
    Optional<Pattern*> pattern;
    Optional<Identifier> name;

    void print(std::ostream& out) const override;
};

struct MatchOr: public Pattern {
    Array<Pattern*> patterns;

    void print(std::ostream& out) const override;
};

struct MatchCase {
    Pattern* pattern;
    Optional<ExprNode*> guard;
    Array<StmtNode*> body;

    void print(std::ostream& out, int indent) const;
};

// Expressions
// -----------

struct BoolOp: public ExprNode{
    BoolOperator op;
    Array<ExprNode*> values;

};
struct NamedExpr: public ExprNode{
    ExprNode* target = nullptr;
    ExprNode* value = nullptr;
};

struct BinOp: public ExprNode{
    ExprNode* left = nullptr;
    BinaryOperator op;
    ExprNode* right = nullptr;
};

struct UnaryOp: public ExprNode{
    UnaryOperator op;
    ExprNode* operand;
};

struct Lambda: public ExprNode{
    Arguments args;
    ExprNode* body = nullptr;

    void print(std::ostream& out, int indent) const ;
};

struct IfExp: public ExprNode{
    ExprNode* test = nullptr;
    ExprNode* body = nullptr;
    ExprNode* orelse = nullptr;

    void print(std::ostream& out, int indent) const ;
};

struct DictExpr: public ExprNode{
    Array<ExprNode*> keys;
    Array<ExprNode*> values;

    void print(std::ostream& out, int indent) const ;
};

struct SetExpr: public ExprNode{
    Array<ExprNode*> elts;

    void print(std::ostream& out, int indent) const;
};

struct ListComp: public ExprNode{
    ExprNode* elt = nullptr;
    Array<Comprehension> generators;

    void print(std::ostream& out, int indent) const;
};

struct GeneratorExp: public ExprNode{
    ExprNode* elt = nullptr;
    Array<Comprehension> generators;

    void print(std::ostream& out, int indent) const;
};

struct SetComp: public ExprNode{
    ExprNode* elt = nullptr;
    Array<Comprehension> generators;

    void print(std::ostream& out, int indent) const;
};

struct DictComp: public ExprNode{
    ExprNode* key = nullptr;
    ExprNode* value = nullptr;
    Array<Comprehension> generators;

    void print(std::ostream& out, int indent) const;
};


// the grammar constrains where yield expressions can occur
struct Await: public ExprNode{
    ExprNode* value;

    void print(std::ostream& out, int indent) const;
};

struct Yield: public ExprNode{
    Optional<ExprNode*> value;

    void print(std::ostream& out, int indent) const;
};

struct YieldFrom: public ExprNode{
    ExprNode* value = nullptr;

    void print(std::ostream& out, int indent) const;
};

// need sequences for compare to distinguish between
// x < 4 < 3 and (x < 4) < 3
struct Compare: public ExprNode{
    ExprNode* left = nullptr;
    Array<CmpOperator> ops;
    Array<ExprNode*> comparators;
};

struct Call: public ExprNode{
    ExprNode* func = nullptr;
    Array<ExprNode*> args;
    Array<Keyword> keywords;

    void print(std::ostream &out, int indent) const override;
};

struct JoinedStr: public ExprNode{
    Array<ExprNode*> values;
};

struct FormattedValue: public ExprNode{
    ExprNode* value = nullptr;
    Optional<ConversionKind> conversion = ConversionKind::None;
    // defined as ExprNode*
    JoinedStr format_spec;
};

struct Constant: public ExprNode{
    ConstantValue value;
    Optional<String> kind;

    void print(std::ostream &out, int indent) const override;
};

 // the following expression can appear in assignment context
struct Attribute: public ExprNode{
    ExprNode* value = nullptr;
    Identifier attr;
    ExprContext ctx;

    void print(std::ostream &out, int indent) const override {
        value->print(out, indent);
        out << ".";
        out << attr;
    }
};

struct Subscript: public ExprNode{
    ExprNode* value = nullptr;
    ExprNode* slice = nullptr;
    ExprContext ctx;

    void print(std::ostream &out, int indent) const override {
        value->print(out, indent);
        out << "[";
        slice->print(out, indent);
        out << "]";
    }
};

struct Starred: public ExprNode{
    ExprNode* value = nullptr;
    ExprContext ctx;

    void print(std::ostream &out, int indent) const override {
        out << "*";
        value->print(out, indent);
    }
};

struct Name: public ExprNode{
    Identifier id;
    ExprContext ctx;

    void print(std::ostream &out, int indent) const override {
        out << id;
    }
};

struct ListExpr: public ExprNode{
    Array<ExprNode*> elts;
    ExprContext ctx;

    void print(std::ostream& out, int indent) const;
};

struct TupleExpr: public ExprNode{
    Array<ExprNode*> elts;
    ExprContext ctx;

    void print(std::ostream& out, int indent) const;
};

// can appear only in Subscript
struct Slice: public ExprNode {
    Optional<ExprNode*> lower;
    Optional<ExprNode*> upper;
    Optional<ExprNode*> step;

    void print(std::ostream& out, int indent) const;
};

// Modules
// -------
struct Module: public ModNode {
    Array<StmtNode*> body;

    String docstring;
};

struct Interactive: public ModNode {
    Array<StmtNode*> body;
};

struct Expression: public ModNode {
    ExprNode* body = nullptr;
};

struct FunctionType: public ModNode {
    Array<ExprNode*> argtypes;
    ExprNode* returns = nullptr;
};

// Statements
// ----------
struct FunctionDef: public StmtNode {
    Identifier name;
    Arguments args;
    Array<StmtNode*> body;
    Array<ExprNode*> decorator_list;
    Optional<ExprNode*> returns;
    String type_comment;

    String docstring;
    bool async: 1 = false;

    void print(std::ostream &out, int indent) const override;
};

struct AsyncFunctionDef: public FunctionDef {
};

struct ClassDef: public StmtNode {
    Identifier name;
    Array<ExprNode*> bases;
    Array<Keyword> keywords;
    Array<StmtNode*> body;
    Array<ExprNode*> decorator_list;

    String docstring;

    void print(std::ostream &out, int indent) const override;
};

struct Return: public StmtNode {
    Optional<ExprNode*> value;

    void print(std::ostream& out, int indent) const;
};

struct Delete: public StmtNode {
    Array<ExprNode*> targets;

    void print(std::ostream& out, int indent) const;
};

struct Assign: public StmtNode {
    Array<ExprNode*> targets;
    ExprNode* value = nullptr;
    Optional<String> type_comment;

    void print(std::ostream& out, int indent) const;
};

struct AugAssign: public StmtNode{
    ExprNode* target = nullptr;
    BinaryOperator op;
    ExprNode* value = nullptr;
};

// 'simple' indicates that we annotate simple name without parens
struct AnnAssign: public StmtNode {
    ExprNode* target = nullptr;
    ExprNode* annotation = nullptr;
    Optional<ExprNode*> value;
    int simple;

    void print(std::ostream& out, int indent) const;
};

// use 'orelse' because else is a keyword in target languages
struct For: public StmtNode {
    ExprNode* target = nullptr;
    ExprNode* iter = nullptr;
    Array<StmtNode*> body;
    Array<StmtNode*> orelse;
    Optional<String> type_comment;

    bool async = false;
};

// Keeping it for consistency with python docs, but useless
struct AsyncFor: public For {

};

struct While: public StmtNode {
    ExprNode* test = nullptr;
    Array<StmtNode*> body;
    Array<StmtNode*> orelse;
};

struct If: public StmtNode {
    ExprNode* test = nullptr;
    Array<StmtNode*> body;
    Array<StmtNode*> orelse;
};

struct With: public StmtNode {
    Array<WithItem> items;
    Array<StmtNode*> body;
    Optional<String> type_comment;

    bool async = false;
};

// Keeping it for consistency with python docs, but useless
struct AsyncWith: public With {

};

struct Raise: public StmtNode {
    Optional<ExprNode*> exc;
    Optional<ExprNode*> cause;
};

struct Try: public StmtNode {
    Array<StmtNode*> body;
    Array<ExceptHandler> handlers;
    Array<StmtNode*> orelse;
    Array<StmtNode*> finalbody;
};

struct Assert: public StmtNode {
    ExprNode* test = nullptr;
    Optional<ExprNode*> msg;
};

struct Import: public StmtNode {
    Array<Alias> names;
};

struct ImportFrom: public StmtNode {
    Optional<Identifier> module;
    Array<Alias> names;
    Optional<int> level;
};

struct Global: public StmtNode {
    Array<Identifier> names;

    void print(std::ostream& out, int indent) const;
};

struct Nonlocal: public StmtNode {
    Array<Identifier> names;

    void print(std::ostream& out, int indent) const;
};

struct Expr: public StmtNode {
    ExprNode* value = nullptr;

    void print(std::ostream& out, int indent) const;
};

struct Pass: public StmtNode {
    void print(std::ostream& out, int indent) const;
};

struct Break: public StmtNode {
    void print(std::ostream& out, int indent) const;
};

struct Continue: public StmtNode {
    void print(std::ostream& out, int indent) const;
};

struct Match: public StmtNode {
    ExprNode* subject;
    Array<MatchCase> cases;

    void print(std::ostream& out, int indent) const;
};

//
struct NotImplementedStmt: public StmtNode {
    void print(std::ostream& out, int indent) const {
        out << "<not implemented>";
    }
};

struct NotImplementedExpr: public ExprNode {
    void print(std::ostream& out, int indent) const {
        out << "<not implemented>";
    }
};

struct NotAllowedEpxr: public ExprNode {
    String msg;

    void print(std::ostream& out, int indent) const {
        out << "<not allowed: " << msg << ">";
    }
};

}
#endif
