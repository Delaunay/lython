#ifndef LYTHON_SEMA_HEADER
#define LYTHON_SEMA_HEADER

#include "ast/visitor.h"

namespace lython {

using ExprType = ExprNode;

struct SemanticAnalyser: BaseVisitor<SemanticAnalyser> {
    public:
    Module *module(Module *stmt, int depth) {
        exec(stmt->body, depth);
        return nullptr;
    };

    void enter();
    void exit();

    // returns the varid it was inserted as
    int add(StringRef name, Node *value, ExprType *type) {}

    void set_type(int varid, ExprType *type) {}

    ExprType *get_type(int varid) {}

    ExprNode *get_value(int varid) {}

    StringRef get_name(int varid) {}

    int get_varid(StringRef name) {}

    template <typename T>
    ExprType *exec_body(Array<T> &body, int depth) {
        Array<ExprType *> types;

        for (auto &stmt: body) {
            auto t = exec(stmt, depth);
            if (t != nullptr) {
                types.push_back(t);
            }
        }

        if (types.size() <= 0)
            return nullptr;

        if (types.size() == 1)
            return types[0];

        // TODO: do check that all types matches
        return types[0];
    }

#define FUNCTION_GEN(name, fun) ExprType *fun(name *n, int depth);

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