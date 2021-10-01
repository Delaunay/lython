#ifndef LYTHON_AST_VISITOR_HEADER
#define LYTHON_AST_VISITOR_HEADER

#include "ast/sexpression.h"

namespace lython {

NEW_EXCEPTION(NullPointerError)

/*!
 * Visitor implemented using static polymorphism.
 * This implementation has 2 advantages:
 *  1. Enable better inlining & devirtualization than vtables
 *      - More cache friendly
 *  2. Reduce the number of indirection from 2 to 1
 *      - 2 because of the double vtable lookup (visitor vtable + node vtable)
 *
 * The main drawbacks is it uses macros to generate the vtable at compile time
 * and the error messages can be a bit arcane
 */
template <typename Implementation, typename... Args>
struct BaseVisitor {

    Array<StmtNode *> exec(Array<StmtNode *> &body, int depth, Args... args) {
        int k = 0;
        for (auto &stmt: body) {
            body[k] = exec(stmt, depth, std::forward(args)...);
            k += 1;
        }
        return body;
    };

    ModNode *exec(ModNode *mod, int depth, Args... args) {
        // clang-format off
        switch (mod->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SECTION(_)
            #define MOD(name, fun)\
                case NodeKind::name: {\
                    name* m = reinterpret_cast<name*>(mod);\
                    return fun(m, depth + 1, std::forward(args)...);\
                }

            NODEKIND_ENUM(X, SECTION, PASS, PASS, MOD, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef MOD
        
        }
        // clang-format on
        return nullptr;
    }

    Pattern *exec(Pattern *pat, int depth, Args... args) {
        // clang-format off
        switch (pat->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SECTION(_)
            #define MATCH(name, fun)\
                case NodeKind::name: {\
                    name* p = reinterpret_cast<name*>(pat);\
                    return fun(p, depth + 1, std::forward(args)...);\
                }

            NODEKIND_ENUM(X, SECTION, PASS, PASS, PASS, MATCH)

            #undef X
            #undef PASS
            #undef SECTION
            #undef MATCH
        
        }
        // clang-format on
        return nullptr;
    }

    ExprNode *exec(ExprNode *expr, int depth, Args... args) {
        // clang-format off
        switch (expr->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SECTION(_)
            #define EXPR(name, fun)\
                case NodeKind::name: {\
                    name* node = reinterpret_cast<name*>(expr);\
                    return fun(node, depth + 1, std::forward(args)...);\
                }

            NODEKIND_ENUM(X, SECTION, EXPR, PASS, PASS, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef EXPR
        
        }
        // clang-format on
        return nullptr;
    }

    StmtNode *exec(StmtNode *stmt, int depth, Args... args) {
        // clang-format off
        switch (stmt->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SECTION(_)
            #define STMT(name, fun)\
                case NodeKind::name: {\
                    name* stmt = reinterpret_cast<name*>(stmt);\
                    return fun(stmt, depth + 1, std::forward(args)...);\
                }

            NODEKIND_ENUM(X, SECTION, PASS, STMT, PASS, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef STMT
        
        }
        // clang-format on
        return nullptr;
    }

#define FUNCTION_GEN(name, fun)                                                              \
    name *fun(name *node, int depth, Args... args) {                                         \
        return static_cast<Implementation *>(this)->fun(node, depth, std::forward(args)...); \
    }

#define X(name, _)
#define SECTION(name)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun)
#define STMT(name, fun)  FUNCTION_GEN(name, fun)
#define MOD(name, fun)   FUNCTION_GEN(name, fun)
#define MATCH(name, fun) FUNCTION_GEN(name, fun)

    NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef FUNCTION_GEN

}; // namespace lython

} // namespace lython
#endif
