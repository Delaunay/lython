#ifndef LYTHON_SEMA_HEADER
#define LYTHON_SEMA_HEADER

#include "ast/magic.h"
#include "ast/visitor.h"
#include "utilities/strings.h"

namespace lython {

using TypeExpr = ExprNode;

struct BindingEntry {
    BindingEntry(StringRef a = StringRef(), Node *b = nullptr, TypeExpr *c = nullptr):
        name(a), value(b), type(c) {}

    bool operator==(BindingEntry const &b) const {
        return name == b.name && value == b.value && type == b.type;
    }

    StringRef name;
    Node *    value = nullptr;
    TypeExpr *type  = nullptr;
};

std::ostream &print(std::ostream &out, BindingEntry const &entry);

struct Bindings {
    // returns the varid it was inserted as
    inline int add(StringRef const &name, Node *value, TypeExpr *type) {
        auto size = int(bindings.size());
        bindings.push_back({name, value, type});
        return size;
    }

    inline void set_type(int varid, TypeExpr *type) {
        if (varid < 0 && varid > bindings.size())
            return;

        bindings[varid].type = type;
    }

    inline TypeExpr *get_type(int varid) {
        if (varid < 0 && varid > bindings.size())
            return nullptr;
        return bindings[varid].type;
    }

    inline Node *get_value(int varid) {
        if (varid < 0 && varid > bindings.size())
            return nullptr;
        return bindings[varid].value;
    }

    StringRef get_name(int varid) {
        if (varid < 0 && varid > bindings.size())
            return StringRef();
        return bindings[varid].name;
    }

    int get_varid(StringRef name) {
        auto start = std::rbegin(bindings);
        auto end   = std::rend(bindings);

        int i = 0;
        while (start != end) {
            if (start->name == name) {
                return i;
            }
            ++start;
            i += 1;
        }
        return -1;
    }

    String __str__() const {
        StringStream ss;
        dump(ss);
        return ss.str();
    }

    void dump(std::ostream &out) const;

    Array<BindingEntry> bindings;
};

struct Scope {
    Scope(Bindings &array): bindings(array), oldsize(bindings.bindings.size()) {}

    ~Scope() { bindings.bindings.resize(oldsize); }

    Bindings &  bindings;
    std::size_t oldsize;
};

struct SemanticError {
    StmtNode *   stmt;
    ExprNode *   expr;
    Pattern *    pat;
    String       message;
    CodeLocation loc;
};

struct SemaVisitorTrait {
    using StmtRet = TypeExpr;
    using ExprRet = TypeExpr;
    using ModRet  = TypeExpr;
    using PatRet  = TypeExpr;
};

/* The semantic analysis (SEM-A) happens after the parsing, the AST can be assumed to be
 * syntactically correct its job is to detect issues that could prevent a succesful compilation.
 *
 * Errors caught in that process are undeclared variables and mistypings,
 * this includes missing attributes, missing methods
 *
 * In addition, our SEM-A will deduce types (i.e variables inherit the type of the expressions,
 * this is NOT type inference) and allocate a register to each variables.
 *
 * To support type deduction SEM-A returns the type of the analysed expression,
 * the deduction can then be used for typechecking.
 *
 * Type deduction is a weaker form of type inference where the type of the parent parent expression
 * is deduced from the children. In the future we might add full type inference.
 * Type deduction will still be useful then as it will reduce the cost of type inference for the
 * trivial cases.
 *
 * Type deduction alone should provide a satisfactory development experience, as the user should
 * only have to specify the type of the arguments which is good practice anyway as it serves as
 * documentation.
 */
struct SemanticAnalyser: BaseVisitor<SemanticAnalyser, SemaVisitorTrait> {
    Bindings             bindings;
    bool                 forwardpass = false;
    Array<SemanticError> errors;

    public:
    virtual ~SemanticAnalyser() {}

    bool typecheck(TypeExpr *one, TypeExpr *two) { return true; }

    bool add_name(ExprNode *expr, ExprNode *value, ExprNode *type);

    TypeExpr *oneof(Array<TypeExpr *> types) {
        if (types.size() > 0) {
            return types[0];
        }
        return nullptr;
    }

    TypeExpr *module(Module *stmt, int depth) {
        exec<TypeExpr>(stmt->body, depth);
        return nullptr;
    };

    void add_arguments(Arguments &args, Arrow *, int);

#define FUNCTION_GEN(name, fun) virtual TypeExpr *fun(name *n, int depth);

#define X(name, _)
#define SECTION(name)
#define MOD(name, fun)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun)
#define STMT(name, fun)  FUNCTION_GEN(name, fun)
#define MATCH(name, fun) FUNCTION_GEN(name, fun)

    NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef FUNCTION_GEN
};

} // namespace lython

#endif