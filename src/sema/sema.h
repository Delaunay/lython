#ifndef LYTHON_SEMA_HEADER
#define LYTHON_SEMA_HEADER

#include "ast/visitor.h"

namespace lython {

struct SemanticAnalyser: BaseVisitor<SemanticAnalyser> {
    public:
    Module *module(Module *stmt, int depth) {
        exec(stmt->body, depth);
        return nullptr;
    };

#define FUNCTION_GEN(name, fun) \
    name *fun(name *n, int depth) { return n; }

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