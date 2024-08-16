#ifndef LYTHON_SEXPR_HEADER
#define LYTHON_SEXPR_HEADER

#include <memory>

#include "kmeta.h"
#include "ast/nodekind.h"
#include "dtypes.h"
#include "lexer/token.h"
#include "logging/logging.h"
#include "utilities/names.h"
#include "utilities/object.h"
#include "utilities/optional.h"
#include "values/value.h"





namespace lython {

using Identifier = StringRef;

String str(NodeKind k);

enum class NodeFamily : int8_t
{
    Module,
    Statement,
    Expression,
    Pattern,
    VM,
};


// col_offset is the byte offset in the utf8 string the parser uses
KSTRUCT(a) 
struct CommonAttributes {
    int           lineno     = -2;
    int           col_offset = -2;
    Optional<int> end_lineno;

    KPROPERTY(b) 
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
    // String __str__() const;

    Node(NodeKind _kind): kind(_kind) {}

    virtual NodeFamily family() const = 0;

    const NodeKind kind;

    template <typename T>
    bool is_instance() const {
        return kind == nodekind<T>();
    }

    virtual bool is_leaf() = 0;

    Node const* get_parent() const { return static_cast<Node*>(get_gc_parent()); }
};

struct ModNode: public Node {
    ModNode(NodeKind kind): Node(kind) {}

    NodeFamily family() const override { return NodeFamily::Module; }

    bool is_leaf() override { return false; }
};

struct Comment;

struct StmtNode: public CommonAttributes, public Node {
    StmtNode(NodeKind kind): Node(kind) {}

    bool is_leaf() override { return false; }

    NodeFamily family() const override { return NodeFamily::Statement; }

    // inline comments are inserted to its matching statement
    // example:
    //      <stmt> # comment
    //
    // Some statements can have multiple comment
    //
    // if <expr>: # comment
    //     ...
    // else: # commnet
    //
    Comment* comment = nullptr;

    bool is_one_line() const {
        if (end_lineno.has_value()) {
            return lineno == end_lineno.value();
        }
        return true;
    }
};

struct ExprNode: public CommonAttributes, public Node {
    ExprNode(NodeKind kind): Node(kind) {}

    NodeFamily family() const override { return NodeFamily::Expression; }

    bool is_leaf() override { return false; }
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
#define BINARY_OPERATORS(OP)     \
    OP(None, "", na)             \
    OP(Add, "+", add)            \
    OP(Sub, "-", sub)            \
    OP(Mult, "*", mul)           \
    OP(MatMult, "@", matmul)     \
    OP(Div, "/", truediv)        \
    OP(Mod, "%", divmod)         \
    OP(Pow, "^", pow)            \
    OP(LShift, "<<", lshift)     \
    OP(RShift, ">>", rshift)     \
    OP(BitOr, "|", or)           \
    OP(BitXor, "^", xor)         \
    OP(BitAnd, "&", and)         \
    OP(FloorDiv, "//", floordiv) \
    OP(EltMult, ".*", eltmult)   \
    OP(EltDiv, "./", eltiv)

#define OP(name, _, n) name,
    BINARY_OPERATORS(OP)
#undef OP
};

enum class BoolOperator : int8_t
{
    None,
#define BOOL_OPERATORS(OP) \
    OP(And, and, and)      \
    OP(Or, or, or)

#define OP(name, kw, _) name,
    BOOL_OPERATORS(OP)
#undef OP
};

enum class UnaryOperator : int8_t
{
#define UNARY_OPERATORS(OP) \
    OP(None, "", na)        \
    OP(Invert, "~", invert) \
    OP(Not, "!", not )      \
    OP(UAdd, "+", pos)      \
    OP(USub, "-", neg)

#define CONV(OP)                    \
    OP(abs, "abs", abs)             \
    OP(int, "int", int)             \
    OP(float, "float", float)       \
    OP(index, "index", index)       \
    OP(complex, "complex", complex) \
    OP(round, "round", round)       \
    OP(trunc, "trunc", trunc)       \
    OP(floor, "floor", floor)       \
    OP(ceil, "ceil", ceil)

#define OP(name, kw, n) name,
    UNARY_OPERATORS(OP)
#undef OP
};

enum class CmpOperator : int8_t
{

#define COMP_OPERATORS(OP)     \
    OP(None, "", na)           \
    OP(Eq, "==", eq)           \
    OP(NotEq, "!=", ne)        \
    OP(Lt, "<", lt)            \
    OP(LtE, "<=", le)          \
    OP(Gt, ">", gt)            \
    OP(GtE, ">=", ge)          \
    OP(Is, "is", is)           \
    OP(IsNot, "is not", isnot) \
    OP(In, "in", in)           \
    OP(NotIn, "not in", notin)

#define OP(name, _, n) name,
    COMP_OPERATORS(OP)
#undef OP
};

enum class ExprContext : int8_t
{
    Load,
    Store,
    LoadStore,
    Del
};

std::ostream& operator<<(std::ostream& out, UnaryOperator const& v) ;
std::ostream& operator<<(std::ostream& out, BinaryOperator const& v) ;
std::ostream& operator<<(std::ostream& out, BoolOperator const& v) ;
std::ostream& operator<<(std::ostream& out, CmpOperator const& v) ;

// stp
StringRef operator_magic_name(BoolOperator const& v, bool reverse = false);
StringRef operator_magic_name(BinaryOperator const& v, bool reverse = false);
StringRef operator_magic_name(CmpOperator const& v, bool reverse = false);
StringRef operator_magic_name(UnaryOperator const& v, bool reverse = false);
// test

struct Comprehension {
    ExprNode*        target = nullptr;
    ExprNode*        iter   = nullptr;
    Array<ExprNode*> ifs;
    int              is_async; // : 1;
};

struct ExceptHandler: public CommonAttributes {
    Optional<ExprNode*>  type;
    Optional<Identifier> name;
    Array<StmtNode*>     body;
    Comment*             comment = nullptr;
};

struct Arg: public CommonAttributes {
    Identifier          arg = Identifier();
    Optional<ExprNode*> annotation;
    Optional<String>    type_comment;
};


enum class ArgumentKind {
    PosOnly,
    Regular,
    KwOnly,
    VarArg,
    KwArg,
    None,
};

template<bool IsConst>
struct ArgumentIter {
    using Epxr_t = typename std::conditional<IsConst, ExprNode const*, ExprNode*>::type;
    using Arg_t = typename std::conditional<IsConst, Arg const&, Arg&>::type;

    ArgumentKind kind;
    Arg_t arg;
    Epxr_t value;
};

struct Arguments {
    Arguments() = default;

    Arguments(Arguments const&) = default;

    Array<Arg>       posonlyargs;
    Array<Arg>       args;
    Optional<Arg>    vararg;  // *args      -> This could be resolved at the call site
    Array<Arg>       kwonlyargs;
    Array<ExprNode*> kw_defaults;
    Optional<Arg>    kwarg;  // **kwargs    -> This could be resolved at the call site
    Array<ExprNode*> defaults;

    // we should probably have some structure to record the resolved variadic arguments
    // could be a superset here
    // or maybe generate some code for a subset at the call site

    // f(*args, **kwargs) <= This is just forward the arguments as is
    bool is_forward() {
        return posonlyargs.size() == 0 && args.size() == 0 && 
            vararg.has_value() && kwonlyargs.size() == 0 &&
            kw_defaults.size() == 0 && kwarg.has_value() && defaults.size() == 0;
    }

    bool is_variadic() {
        return vararg.has_value() || kwarg.has_value();
    }

    int size() const { return int(posonlyargs.size() + args.size() + kwonlyargs.size()); }

    // template<typename Fun>
    // void visit(Fun fun) {
    //     static_visit(this, fun);
    // }

    // template<typename Fun>
    // void visit(Fun fun) const {
    //     static_visit(this, fun);
    // }

    KIGNORE()
    void visit(std::function<void(ArgumentIter<false> const&)> fun); 

    KIGNORE()
    void visit(std::function<void(ArgumentIter<true> const&)> fun) const;
};

struct Keyword: public CommonAttributes {
    Identifier arg;  // why is this optional ?
                     // it is marked as optional in the python AST
    ExprNode* value = nullptr;
};

struct Alias {
    Identifier           name;
    Optional<Identifier> asname;
};

struct WithItem {
    ExprNode*           context_expr = nullptr;
    Optional<ExprNode*> optional_vars;
};

struct TypeIgnore {
    int    lineno;
    String tag;
};

struct Pattern: public CommonAttributes, public Node {
    Pattern(NodeKind kind): Node(kind) {}

    NodeFamily family() const override { return NodeFamily::Pattern; }

    bool is_leaf() override { return false; }
};

struct MatchValue: public Pattern {
    ExprNode* value;

    MatchValue(): Pattern(NodeKind::MatchValue) {}
};

struct MatchSingleton: public Pattern {
    Value value;
    ValueDeleter deleter = nullptr;

    ~MatchSingleton() {
        if (deleter) {
            deleter(nullptr, value);
        }
    }

    MatchSingleton(): Pattern(NodeKind::MatchSingleton) {}
};

struct MatchSequence: public Pattern {
    Array<Pattern*> patterns;

    MatchSequence(): Pattern(NodeKind::MatchSequence) {}
};

// The optional "rest" MatchMapping parameter handles capturing extra mapping keys
struct MatchMapping: public Pattern {
    Array<ExprNode*>     keys;
    Array<Pattern*>      patterns;
    Optional<Identifier> rest;

    MatchMapping(): Pattern(NodeKind::MatchMapping) {}
};

struct MatchClass: public Pattern {
    ExprNode*         cls;
    Array<Pattern*>   patterns;
    Array<Identifier> kwd_attrs;
    Array<Pattern*>   kwd_patterns;

    MatchClass(): Pattern(NodeKind::MatchClass) {}
};

struct MatchStar: public Pattern {
    Optional<Identifier> name;

    MatchStar(): Pattern(NodeKind::MatchStar) {}
};

struct MatchAs: public Pattern {
    Optional<Pattern*>   pattern;
    Optional<Identifier> name;

    MatchAs(): Pattern(NodeKind::MatchAs) {}
};

struct MatchOr: public Pattern {
    Array<Pattern*> patterns;

    MatchOr(): Pattern(NodeKind::MatchOr) {}
};

struct MatchCase {
    Pattern*            pattern;
    Optional<ExprNode*> guard;
    Array<StmtNode*>    body;
    Comment*            comment = nullptr;
};

// Expressions
// -----------

// So currently comment are tokens
// maybe it should be its own single string token
// so I do not have to worry about the formatting
//
// currently the tokens are formatted back using the unlex
// which tries to output tokens following python code style
struct Comment: public ExprNode {

    Comment(): ExprNode(NodeKind::Comment) {}

    String comment;
};

struct Constant: public ExprNode {
    Value        value;
    ValueDeleter deleter = nullptr;
    Optional<String> kind;

    template <typename T>
    Constant(T const& v): ExprNode(NodeKind::Constant)
    {
        value = make_value<T>(v);
        // kwassert(deleter != nullptr, "deleter needs to be valid");
    }

    Constant(Value v): ExprNode(NodeKind::Constant), value(v)
    {
    }

    Constant(): ExprNode(NodeKind::Constant){}

    ~Constant() {
        if (deleter) {
            deleter(nullptr, value);
        }
        deleter = nullptr;
    }

    bool is_leaf() { return true; }
};

// Dummy, expression representing a value to be pluged at runtime
struct Placeholder: public ExprNode {
    Placeholder(): ExprNode(NodeKind::Placeholder) {}

    ExprNode* expr;

    bool is_leaf() { return true; }
};

/*
    >>> print(ast.dump(ast.parse("1 < 2 < 3"), indent=4))
    Module(
        body=[
            Expr(
                value=Compare(
                    left=Constant(value=1),
                    ops=[
                        Lt(),
                        Lt()],
                    comparators=[
                        Constant(value=2),
                        Constant(value=3)]))],
        type_ignores=[])
 */
struct BoolOp: public ExprNode {
    BoolOperator     op;
    Array<ExprNode*> values;

    // this is used to know if we have a partial expression or not
    // if the expression is valid we should have values == opcount + 1
    int opcount = 0;

    // Function to apply, resolved by the sema
    StmtNode*     resolved_operator = nullptr;
    Function      native_operator   = nullptr;
    int           varid             = -1;

    BoolOp(): ExprNode(NodeKind::BoolOp) {}

    bool safe_value_add(ExprNode* value) {
        if (value == this) {
            return false;
        }

        for (auto& expr: values) {
            if (value == expr) {
                return false;
            }
        }

        values.push_back(value);
        return true;
    }
};

// need sequences for compare to distinguish between
// x < 4 < 3 and (x < 4) < 3
struct Compare: public ExprNode {
    ExprNode*          left = nullptr;
    Array<CmpOperator> ops;
    Array<ExprNode*>   comparators;

    // Function to apply, resolved by the sema
    Array<StmtNode*> resolved_operator;
    Array<Function>  native_operator;

    bool safe_comparator_add(ExprNode* comp) {
        if (comp == this) {
            return false;
        }

        for (auto& expr: comparators) {
            if (comp == expr) {
                return false;
            }
        }

        comparators.push_back(comp);
        return true;
    }

    Compare(): ExprNode(NodeKind::Compare) {}
};

struct NamedExpr: public ExprNode {
    ExprNode* target = nullptr;
    ExprNode* value  = nullptr;

    NamedExpr(): ExprNode(NodeKind::NamedExpr) {}

    bool is_leaf() override { return true; }
};

struct BinOp: public ExprNode {
    ExprNode*      left = nullptr;
    BinaryOperator op;
    ExprNode*      right = nullptr;

    // Function to apply, resolved by the sema
    StmtNode*      resolved_operator = nullptr;
    Function       native_operator   = nullptr;
    int            varid             = -1;

    BinOp(): ExprNode(NodeKind::BinOp) {}
};

struct UnaryOp: public ExprNode {
    UnaryOperator op;
    ExprNode*     operand;

    // Function to apply, resolved by the sema
    StmtNode* resolved_operator = nullptr;
    Function  native_operator   = nullptr;

    UnaryOp(): ExprNode(NodeKind::UnaryOp) {}
};

struct Lambda: public ExprNode {
    Arguments args;
    ExprNode* body = nullptr;

    Lambda(): ExprNode(NodeKind::Lambda) {}
};

struct IfExp: public ExprNode {
    ExprNode* test   = nullptr;
    ExprNode* body   = nullptr;
    ExprNode* orelse = nullptr;

    IfExp(): ExprNode(NodeKind::IfExp) {}
};

struct DictExpr: public ExprNode {
    Array<ExprNode*> keys;
    Array<ExprNode*> values;

    DictExpr(): ExprNode(NodeKind::DictExpr) {}
};

struct SetExpr: public ExprNode {
    Array<ExprNode*> elts;

    SetExpr(): ExprNode(NodeKind::SetExpr) {}
};

struct ListComp: public ExprNode {
    ExprNode*            elt = nullptr;
    Array<Comprehension> generators;

    ListComp(): ExprNode(NodeKind::ListComp) {}
};

struct GeneratorExp: public ExprNode {
    ExprNode*            elt = nullptr;
    Array<Comprehension> generators;

    GeneratorExp(): ExprNode(NodeKind::GeneratorExp) {}
};

struct SetComp: public ExprNode {
    ExprNode*            elt = nullptr;
    Array<Comprehension> generators;

    SetComp(): ExprNode(NodeKind::SetComp) {}
};

struct DictComp: public ExprNode {
    ExprNode*            key   = nullptr;
    ExprNode*            value = nullptr;
    Array<Comprehension> generators;

    DictComp(): ExprNode(NodeKind::DictComp) {}
};

// the grammar constrains where yield expressions can occur
struct Await: public ExprNode {
    ExprNode* value;

    Await(): ExprNode(NodeKind::Await) {}
};

#undef Yield
struct Yield: public ExprNode {
    Optional<ExprNode*> value;

    Yield(): ExprNode(NodeKind::Yield) {}
};

struct YieldFrom: public ExprNode {
    ExprNode* value = nullptr;

    YieldFrom(): ExprNode(NodeKind::YieldFrom) {}
};

struct Call: public ExprNode {
    ExprNode*        func = nullptr;
    Array<ExprNode*> args;
    Array<Keyword>   keywords;
    Array<ExprNode*> varargs;

    int jump_id = -1;

    Call(): ExprNode(NodeKind::Call) {}
};

struct JoinedStr: public ExprNode {
    Array<ExprNode*> values;

    JoinedStr(): ExprNode(NodeKind::JoinedStr) {}
};

struct FormattedValue: public ExprNode {
    ExprNode*                value      = nullptr;
    Optional<ConversionKind> conversion = ConversionKind::None;
    // defined as ExprNode*
    JoinedStr* format_spec;

    FormattedValue(): ExprNode(NodeKind::FormattedValue) {}
};

struct Subscript: public ExprNode {
    ExprNode*   value = nullptr;
    ExprNode*   slice = nullptr;
    ExprContext ctx;

    Subscript(): ExprNode(NodeKind::Subscript) {}
};

struct Starred: public ExprNode {
    ExprNode*   value = nullptr;
    ExprContext ctx;

    Starred(): ExprNode(NodeKind::Starred) {}
};

struct Name: public ExprNode {
    Identifier  id;
    ExprContext ctx;

    // A bit redundant when it comes to AnnAssign
    ExprNode* type = nullptr;

    int store_id = -1;
    int load_id  = -1;

    Name(): ExprNode(NodeKind::Name) {}

    bool is_leaf() override { return true; }
};

struct ListExpr: public ExprNode {
    Array<ExprNode*> elts;
    ExprContext      ctx;

    ListExpr(): ExprNode(NodeKind::ListExpr) {}
};

struct TupleExpr: public ExprNode {
    Array<ExprNode*> elts;
    ExprContext      ctx;

    struct TupleType* type = nullptr;

    // We need the typing of the tuple
    TupleExpr(): ExprNode(NodeKind::TupleExpr) {}
};

// can appear only in Subscript
struct Slice: public ExprNode {
    Optional<ExprNode*> lower;
    Optional<ExprNode*> upper;
    Optional<ExprNode*> step;

    Slice(): ExprNode(NodeKind::Slice) {}
};

// Modules
// -------
struct Module: public ModNode {
    Array<StmtNode*> body;

    Optional<String> docstring;

    struct FunctionDef* __init__ = nullptr;

    Module(): ModNode(NodeKind::Module) {}
};

struct Interactive: public ModNode {
    Array<StmtNode*> body;

    Interactive(): ModNode(NodeKind::Interactive) {}
};

struct Expression: public ModNode {
    ExprNode* body = nullptr;

    Expression(): ModNode(NodeKind::Expression) {}
};

struct FunctionType: public ModNode {
    Array<ExprNode*> argtypes;
    ExprNode*        returns = nullptr;

    FunctionType(): ModNode(NodeKind::FunctionType) {}
};

// Statements
// ----------

// This is used for error recovery
struct InvalidStatement: public StmtNode {
    // FIXME: this node should take ownership of the parsing error
    // struct ParsingError* error = nullptr;

    Array<Token> tokens;

    InvalidStatement(): StmtNode(NodeKind::InvalidStatement) {}
};

struct Inline: public StmtNode {
    // <stmt>; <stmt>
    Array<StmtNode*> body;

    Inline(): StmtNode(NodeKind::Inline) {}
};

struct Decorator {
    ExprNode* expr    = nullptr;
    Comment*  comment = nullptr;

    Decorator(ExprNode* deco=nullptr, Comment* com = nullptr): expr(deco), comment(com) {}
};

struct Docstring {
    String   docstring;
    Comment* comment = nullptr;

    Docstring(String const& doc, Comment* com = nullptr): docstring(doc), comment(com) {}
};

struct FunctionDef: public StmtNode {
    Identifier          name;
    Arguments           args;
    Array<StmtNode*>    body;
    Array<Decorator>    decorator_list = {};
    Optional<ExprNode*> returns;
    String              type_comment;
    Optional<Docstring> docstring;

    bool async; // : 1;
    // SEMA
    bool          generator;// : 1;
    struct Arrow* type = nullptr;

    Function native = nullptr;

    FunctionDef(): StmtNode(NodeKind::FunctionDef), async(false), generator(false) {}
};

struct AsyncFunctionDef: public FunctionDef {};

// This is the AST declaration
// To be able to instantiate an instance of the class we should generate
// a TypeInfo/ClassMetadata
// This is what will enable us to create generate struct and access their members
struct ClassDef: public StmtNode {
    Identifier          name;
    Array<ExprNode*>    bases;
    Array<Keyword>      keywords;
    Array<StmtNode*>    body;
    Array<Decorator>    decorator_list = {};
    Optional<Docstring> docstring;
    int                 type_id = -1;

    Arrow* ctor_t = nullptr;

    ClassDef(): StmtNode(NodeKind::ClassDef) {}

    // Sema populates this, this is for nested classes
    String cls_namespace;

    // To match python AST the body of the class is a simple Array of statement
    // but this is not very convenient for semantic analysis
    //

    struct Attr {
        Attr(StringRef name, int offset = -1, StmtNode* stmt = nullptr, ExprNode* type = nullptr):
            name(name), offset(offset), stmt(stmt), type(type)
        //
        {}

        StringRef name;
        int       offset = -1;
        StmtNode* stmt   = nullptr;
        ExprNode* type   = nullptr;

        operator bool() { return name != StringRef(); }

        KIGNORE()
        void dump(std::ostream& out);
    };
    // Dict<StringRef, Attr> attributes;
    Array<Attr> attributes;  // <= Instantiated Object
    // Array<Attr> static_attributes;  // <= Namespaced Globals
    // Array<Attr> methods;

    KIGNORE()
    void dump(std::ostream& out) {
        out << "Attributes:\n";
        for (auto& item: attributes) {
            out << "`" << item.name << "` ";
            item.dump(out);
            out << "\n";
        }
    }

    int get_attribute(StringRef name) {

        int i = 0;
        for (Attr& att: attributes) {
            if (att.name == name) {
                return i;
            }

            i += 1;
        }

        return -1;
    }

    bool insert_method(StringRef name, StmtNode* stmt = nullptr, ExprNode* type = nullptr) {
        int attrid = get_attribute(name);

        if (attrid == -1) {
            attributes.emplace_back(name, int(attributes.size()), stmt, type);
            return true;
        }

        Attr& v = attributes[attrid];

        if (!v.type && type) {
            v.type = type;
        }

        return false;
    }

    bool insert_attribute(StringRef name, StmtNode* stmt = nullptr, ExprNode* type = nullptr) {
        int attrid = get_attribute(name);

        if (attrid == -1) {
            attributes.emplace_back(name, int(attributes.size()), stmt, type);
            return true;
        }

        Attr& v = attributes[attrid];

        if (v.type == nullptr && type != nullptr) {
            v.type = type;
        }

        return false;
    }
};

// the following expression can appear in assignment context
struct Attribute: public ExprNode {
    ExprNode*   value = nullptr;
    Identifier  attr;
    ExprContext ctx;

    // Node* resolved = nullptr;
    ClassDef::Attr* resolved = nullptr;

    // SEMA
    int attrid = 0;

    Attribute(): ExprNode(NodeKind::Attribute) {}
};

struct Exported: public ExprNode {
    Exported(): ExprNode(NodeKind::Exported) {}

    struct Bindings* source;
    struct Bindings* dest;
    Node*            node;
};

struct Return: public StmtNode {
    Optional<ExprNode*> value;

    Return(): StmtNode(NodeKind::Return) {}
};

struct Delete: public StmtNode {
    Array<ExprNode*> targets;

    Delete(): StmtNode(NodeKind::Delete) {}
};

struct Assign: public StmtNode {
    // The array is useless;
    // only the first element is ever populated ?
    // a, b = c
    // Tuple(a, b) = c
    //
    Array<ExprNode*> targets;
    ExprNode*        value = nullptr;
    Optional<String> type_comment;

    Assign(): StmtNode(NodeKind::Assign) {}
};

struct AugAssign: public StmtNode {
    ExprNode*      target = nullptr;
    BinaryOperator op;
    ExprNode*      value = nullptr;

    // Function to apply, resolved by the sema
    StmtNode*      resolved_operator = nullptr;
    Function       native_operator   = nullptr;

    AugAssign(): StmtNode(NodeKind::AugAssign) {}
};

// 'simple' indicates that we annotate simple name without parens
struct AnnAssign: public StmtNode {
    ExprNode*           target     = nullptr;
    ExprNode*           annotation = nullptr;
    Optional<ExprNode*> value;
    int                 simple;

    AnnAssign(): StmtNode(NodeKind::AnnAssign) {}
};

// use 'orelse' because else is a keyword in target languages
struct For: public StmtNode {
    ExprNode*        target = nullptr;
    ExprNode*        iter   = nullptr;
    Array<StmtNode*> body;
    Array<StmtNode*> orelse;
    Optional<String> type_comment;

    bool async = false;

    For(): StmtNode(NodeKind::For) {}

    Comment* else_comment = nullptr;
};

// Keeping it for consistency with python docs, but useless
struct AsyncFor: public For {};

struct While: public StmtNode {
    ExprNode*        test = nullptr;
    Array<StmtNode*> body;
    Array<StmtNode*> orelse;

    While(): StmtNode(NodeKind::While) {}

    Comment* else_comment = nullptr;
};

struct If: public StmtNode {
    ExprNode*        test = nullptr;
    Array<StmtNode*> body;
    Array<StmtNode*> orelse;

    // alternative representation that diverges from
    // the python ast
    Array<ExprNode*>        tests;
    Array<Array<StmtNode*>> bodies;

    If(): StmtNode(NodeKind::If) {}

    Array<Comment*> tests_comment;
    Comment*        else_comment = nullptr;
};

struct With: public StmtNode {
    Array<WithItem>  items;
    Array<StmtNode*> body;
    Optional<String> type_comment;

    bool async = false;

    With(): StmtNode(NodeKind::With) {}
};

// Keeping it for consistency with python docs, but useless
struct AsyncWith: public With {};

struct Raise: public StmtNode {
    Optional<ExprNode*> exc;
    Optional<ExprNode*> cause;

    Raise(): StmtNode(NodeKind::Raise) {}
};

struct Try: public StmtNode {
    Array<StmtNode*>     body;
    Array<ExceptHandler> handlers;
    Array<StmtNode*>     orelse;
    Array<StmtNode*>     finalbody;

    Try(): StmtNode(NodeKind::Try) {}

    Comment* else_comment    = nullptr;
    Comment* finally_comment = nullptr;
};

struct Assert: public StmtNode {
    ExprNode*           test = nullptr;
    Optional<ExprNode*> msg;

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
    ExprNode* value = nullptr;

    Expr(): StmtNode(NodeKind::Expr) {}

    bool is_leaf() override { return value && value->is_leaf(); }
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
    ExprNode*        subject;
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

// Record a function type as positional arguments only
// makes it easier to re-order the arguments and insert defaults
struct Arrow: public ExprNode {
    Arrow(): ExprNode(NodeKind::Arrow) {}

    // TODO: check how to resolve circular types
    //
    bool add_arg_type(ExprNode* arg_type);
    bool set_arg_type(int i, ExprNode* arg_type);

    Array<StringRef>      names;     // Allow the names to be there as well
    Dict<StringRef, bool> defaults;  //
    ExprNode*             returns = nullptr;

    int arg_count() const { return int(args.size()); }

    Array<ExprNode*> args;
};

struct DictType: public ExprNode {
    DictType(): ExprNode(NodeKind::DictType) {}

    ExprNode* key   = nullptr;
    ExprNode* value = nullptr;
};

struct SetType: public ExprNode {
    SetType(): ExprNode(NodeKind::SetType) {}

    ExprNode* value = nullptr;
};

struct ArrayType: public ExprNode {
    ArrayType(): ExprNode(NodeKind::ArrayType) {}

    ExprNode* value = nullptr;
};

struct TupleType: public ExprNode {
    TupleType(): ExprNode(NodeKind::TupleType) {}

    Array<ExprNode*> types;
};

struct BuiltinType: public ExprNode {
    // using NativeFunction = Constant* (*)(GCObject* root, Array<Constant*> const& args);
    using NativeMacro = ExprNode* (*)(GCObject* root, Array<Node*> const& args);

    BuiltinType(): ExprNode(NodeKind::BuiltinType) {}
    StringRef name;

    NativeMacro native_macro;
};



struct VMNode: public Node {
    VMNode(NodeKind kind):
        Node(kind)
    {}

    bool is_leaf() override {
        return false;
    }

    NodeFamily family() const override { return NodeFamily::VM; }
};

struct VMStmt: public VMNode {
    VMStmt(): VMNode(NodeKind::VMStmt) {}
    StmtNode* stmt = nullptr;
};

struct CondJump: public VMNode {
    CondJump(): VMNode(NodeKind::CondJump) {}

    ExprNode* condition = nullptr;
    int then_jmp = -1;
    int else_jmp = -1;
};

struct Jump: public VMNode {
    Jump(): VMNode(NodeKind::Jump) {}

    int destination = -1;
};

struct VMNativeFunction: public VMNode {
    VMNativeFunction(): VMNode(NodeKind::VMNativeFunction) {}

    Function fun;
};

/*
struct FunctionNative: public StmtNode {
    using WrappedNativeFunction = std::function<Constant*(GCObject*, Array<Constant*> const&)>;
    using NativeFunction = WrappedNativeFunction;

    FunctionNative():
        ExprNode(NodeKind::FunctionNative)
    {}

    StringRef name;
    // Maybe I need a native function Expr/Stmt instead ?
    NativeFunction native_function;
};
*/

// we need that to convert ClassDef which is a statement
// into an expression
//
// Actually: I can use a Name for that
//
struct ClassType: public ExprNode {
    ClassType(): ExprNode(NodeKind::ClassType) {}
    ClassDef* def;
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
#define VM(name, _)    SPECGEN(name)

NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH, VM)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef VM

#undef SPECGEN

// Safe cast
template <typename T>
T* cast(Node* obj) {
    if (obj == nullptr) {
        return nullptr;
    }
    if (obj->is_instance<T>()) {
        return (T*)obj;
    }
    return nullptr;
}

template <typename T>
T const* cast(Node const* obj) {
    if (obj == nullptr) {
        return nullptr;
    }
    if (obj->is_instance<T>()) {
        return (T const*)obj;
    }
    return nullptr;
}

template <typename T>
T* checked_cast(Node* obj) {
    lyassert(
        obj->is_instance<T>(),
        fmt::format("Cast type is not compatible {} != {}", str(obj->kind), str(nodekind<T>())));
    return cast<T>(obj);
}

}  // namespace lython
#endif
