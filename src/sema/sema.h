#ifndef LYTHON_SEMA_HEADER
#define LYTHON_SEMA_HEADER

#include "ast/magic.h"
#include "ast/ops.h"
#include "ast/visitor.h"
#include "sema/bindings.h"
#include "sema/builtin.h"
#include "sema/errors.h"
#include "utilities/strings.h"

#define SEMA_ERROR(exception)      \
    error("{}", exception.what()); \
    errors.push_back(std::unique_ptr<SemaException>(new exception));

namespace lython {

struct SemaVisitorTrait {
    using StmtRet = TypeExpr *;
    using ExprRet = TypeExpr *;
    using ModRet  = TypeExpr *;
    using PatRet  = TypeExpr *;
    using IsConst = std::false_type;
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
 * Type deduction is a weaker form of type inference where the type of the parent parent
 * expression is deduced from the children. In the future we might add full type inference. Type
 * deduction will still be useful then as it will reduce the cost of type inference for the
 * trivial cases.
 *
 * Type deduction alone should provide a satisfactory development experience, as the user should
 * only have to specify the type of the arguments which is good practice anyway as it serves as
 * documentation.
 *
 * Raises
 * ------
 *
 * TypeError
 *      when types between expression mismatch
 *
 * AttributeError
 *      When using an object attribute that does not exist
 *
 * NameError
 *      When using an undefined variable
 *
 */
struct SemanticAnalyser: BaseVisitor<SemanticAnalyser, false, SemaVisitorTrait> {
    Bindings                              bindings;
    bool                                  forwardpass = false;
    Array<std::unique_ptr<SemaException>> errors;
    Array<StmtNode *>                     nested;
    Dict<StringRef, bool>                 flags;
    Array<String>                         paths;

    public:
    virtual ~SemanticAnalyser() {}

    StmtNode *current_namespace() {
        if (nested.size() > 0) {
            return nested[nested.size() - 1];
        }
        return nullptr;
    }

    bool typecheck(ExprNode *lhs, TypeExpr *lhs_t, ExprNode *rhs, TypeExpr *rhs_t,
                   CodeLocation const &loc);

    bool add_name(ExprNode *expr, ExprNode *value, ExprNode *type);

    TypeExpr *oneof(Array<TypeExpr *> types) {
        if (types.size() > 0) {
            return types[0];
        }
        return nullptr;
    }

    TypeExpr *module(Module *stmt, int depth) {
        // TODO: Add a forward pass that simply add functions & variables
        // to the context so the SEMA can look everything up
        exec<TypeExpr *>(stmt->body, depth);
        return nullptr;
    };

    ExprNode *make_ref(Node *parent, String const &name) {
        auto ref   = parent->new_object<Name>();
        ref->id    = name;
        ref->varid = bindings.get_varid(ref->id);
        return ref;
    }

    TypeExpr *attribute_assign(Attribute *n, int depth, TypeExpr *expected);

    void add_arguments(Arguments &args, Arrow *, ClassDef *def, int);

#define FUNCTION_GEN(name, fun) virtual TypeExpr *fun(name *n, int depth);

#define X(name, _)
#define SSECTION(name)
#define MOD(name, fun)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun)
#define STMT(name, fun)  FUNCTION_GEN(name, fun)
#define MATCH(name, fun) FUNCTION_GEN(name, fun)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef FUNCTION_GEN
};

} // namespace lython

#endif