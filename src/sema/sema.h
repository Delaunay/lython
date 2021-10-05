#ifndef LYTHON_SEMA_HEADER
#define LYTHON_SEMA_HEADER

#include "ast/visitor.h"

namespace lython {

using TypeExpr = ExprNode;

struct SemanticAnalyser: BaseVisitor<SemanticAnalyser, TypeExpr, TypeExpr, TypeExpr, TypeExpr> {
    public:
    TypeExpr *module(Module *stmt, int depth) {
        exec<TypeExpr>(stmt->body, depth);
        return nullptr;
    };

#define FUNCTION_GEN(name, fun) TypeExpr *fun(name *n, int depth);

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