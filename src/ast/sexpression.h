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

    void print(std::ostream& out, int indent) const;
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
};

struct MatchClass: public Pattern {
    ExprNode* cls;
    Array<Pattern*> patterns;
    Array<Identifier> kwd_attrs;
    Array<Pattern*> kwd_patterns;
};

struct MatchStar: public Pattern {
    Optional<Identifier> name;
};

struct MatchAs: public Pattern {
    Optional<Pattern*> pattern;
    Optional<Identifier> name;
};

struct MatchOr: public Pattern {
    Array<Pattern*> patterns;
};

struct MatchCase {
    Pattern* pattern;
    Optional<ExprNode*> guard;
    Array<StmtNode*> body;
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
};

struct IfExp: public ExprNode{
    ExprNode* test = nullptr;
    ExprNode* body = nullptr;
    ExprNode* orelse = nullptr;
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
};

struct SetComp: public ExprNode{
    ExprNode* elt = nullptr;
    Array<Comprehension> generators;
};

struct DictComp: public ExprNode{
    ExprNode* key = nullptr;
    ExprNode* value = nullptr;
    Array<Comprehension> generators;
};

struct GeneratorExp: public ExprNode{
    ExprNode* elt = nullptr;
    Array<Comprehension> generators;
};

// the grammar constrains where yield expressions can occur
struct Await: public ExprNode{
    ExprNode* value;
};

struct Yield: public ExprNode{
    Optional<ExprNode*> value;
};

struct YieldFrom: public ExprNode{
    ExprNode* value = nullptr;
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

    void print(std::ostream &out, int indent) const override {
        func->print(out, indent);
        out << "(";
        
        for(int i = 0; i < args.size(); i++){
            args[i]->print(out, indent);

            if (i < args.size() - 1 || keywords.size() > 0)
                out << ", ";
        }

        for(int i = 0; i < keywords.size(); i++){
            out << keywords[i].arg.value();
            out << " = ";
            keywords[i].value->print(out, indent);

            if (i < keywords.size() - 1)
                out << ", ";
        }

        out << ")";
    }
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
    bool async = false;

    void print(std::ostream &out, int indent) const override {
        out << "def " << name << "(";

        args.print(out, indent);
        out << ")";

        if (returns.has_value()) {
            out << " -> "; returns.value()->print(out, indent);
        }

        out << ":\n";

        lython::print(out, indent + 1, body);
    }
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
};

struct Return: public StmtNode {
    Optional<ExprNode*> value;

    void print(std::ostream& out, int indent) const {
        out << "return ";
        
        if (value.has_value()) {
            value.value()->print(out, indent);
        }
    }
};

struct Delete: public StmtNode {
    Array<ExprNode*> targets;

    void print(std::ostream& out, int indent) const {
        out << "del ";
        
        for(int i = 0; i < targets.size(); i++){
            targets[i]->print(out, indent);

            if (i < targets.size() - 1)
                out << ", ";
        }
    }
};

struct Assign: public StmtNode {
    Array<ExprNode*> targets;
    ExprNode* value = nullptr;
    Optional<String> type_comment;

    void print(std::ostream& out, int indent) const {
        targets[0]->print(out, indent);
        out << " = ";
        value->print(out, indent);
    }
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

    void print(std::ostream& out, int indent) const {
        target->print(out, indent);
        out << ": ";
        annotation->print(out, indent);
        if (value.has_value()){
            out << " = ";
            value.value()->print(out, indent);
        }
    }
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
};

struct Nonlocal: public StmtNode {
    Array<Identifier> names;
};

struct Expr: public StmtNode {
    ExprNode* value = nullptr;
};

struct Pass: public StmtNode {
    void print(std::ostream& out, int indent) const {
        out << "pass";
    }
};

struct Break: public StmtNode {
    void print(std::ostream& out, int indent) const {
        out << "break";
    }
};

struct Continue: public StmtNode {
    void print(std::ostream& out, int indent) const {
        out << "continue";
    }
};

struct Match: public StmtNode {
    ExprNode* subject;
    Array<MatchCase> cases;
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
