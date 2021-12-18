#ifndef LYTHON_AST_VISITOR_HEADER
#define LYTHON_AST_VISITOR_HEADER

#include "ast/nodes.h"
#include "logging/logging.h"

namespace lython {

NEW_EXCEPTION(NullPointerError)

struct DefaultVisitorTrait {
    using StmtRet = StmtNode *;
    using ExprRet = ExprNode *;
    using ModRet  = ModNode *;
    using PatRet  = Pattern *;
};

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
template <typename Implementation, bool isConst, typename VisitorTrait, typename... Args>
struct BaseVisitor {

    using StmtRet = typename VisitorTrait::StmtRet;
    using ExprRet = typename VisitorTrait::ExprRet;
    using ModRet  = typename VisitorTrait::ModRet;
    using PatRet  = typename VisitorTrait::PatRet;

#define SELECT_TYPE(T) std::conditional<isConst, T const, T>::type;

    using Node_t     = SELECT_TYPE(Node);
    using ModNode_t  = SELECT_TYPE(ModNode);
    using Pattern_t  = SELECT_TYPE(Pattern);
    using ExprNode_t = SELECT_TYPE(ExprNode);
    using StmtNode_t = SELECT_TYPE(StmtNode);

#undef SELECT_TYPE

#define TYPE_GEN(rtype) using rtype##_t = std::conditional<isConst, rtype const, rtype>::type;

#define X(name, _)
#define SECTION(name)
#define EXPR(name, fun)  TYPE_GEN(name)
#define STMT(name, fun)  TYPE_GEN(name)
#define MOD(name, fun)   TYPE_GEN(name)
#define MATCH(name, fun) TYPE_GEN(name)

    NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef TYPE_GEN

    template <typename U, typename T>
    Array<U> exec(Array<T> &body, int depth, Args... args) {
        int      k = 0;
        Array<U> types;
        for (auto &stmt: body) {
            types.push_back(exec(stmt, depth, (args)...));
            k += 1;
        }
        return types;
    };

    template <typename U, typename T>
    Optional<U> exec(Optional<T> &maybe, int depth, Args... args) {
        if (maybe.has_value()) {
            return some<U>(exec(maybe.value(), depth, (args)...));
        }
        return none<U>();
    };

    template <typename T>
    T exec(Node_t *n, Args... args) {
        switch (n->family()) {
        case NodeFamily::Module:
            return exec(reinterpret_cast<ModNode_t *>(n), 0, (args)...);
        case NodeFamily::Statement:
            return exec(reinterpret_cast<StmtNode_t *>(n), 0, (args)...);
        case NodeFamily::Expression:
            return exec(reinterpret_cast<ExprNode_t *>(n), 0, (args)...);
        case NodeFamily::Pattern:
            return exec(reinterpret_cast<Pattern_t *>(n), 0, (args)...);
        }
        return T();
    }

    ModRet exec(ModNode_t *mod, int depth, Args... args) {
        // clang-format off
        // trace(depth, "{}", mod->kind);  
        switch (mod->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SECTION(_)
            #define MOD(name, fun)\
                case NodeKind::name: {\
                    name##_t* m = reinterpret_cast<name##_t*>(mod);\
                    return fun(m, depth + 1, (args)...);\
                }

            NODEKIND_ENUM(X, SECTION, PASS, PASS, MOD, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef MOD

            default:
                return ModRet();

        }
        // clang-format on
        return ModRet();
    }

    PatRet exec(Pattern_t *pat, int depth, Args... args) {
        if (!pat) {
            return PatRet();
        }
        // trace(depth, "{}", pat->kind);
        // clang-format off
        switch (pat->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SECTION(_)
            #define MATCH(name, fun)\
                case NodeKind::name: {\
                    name##_t* p = reinterpret_cast<name##_t*>(pat);\
                    return fun(p, depth + 1, (args)...);\
                }

            NODEKIND_ENUM(X, SECTION, PASS, PASS, PASS, MATCH)

            #undef X
            #undef PASS
            #undef SECTION
            #undef MATCH

            default:
                return PatRet();
        }
        // clang-format on
        return PatRet();
    }

    ExprRet exec(ExprNode_t *expr, int depth, Args... args) {
        if (!expr) {
            return ExprRet();
        }
        // trace(depth, "{}", expr->kind);
        // clang-format off
        switch (expr->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SECTION(_) 
            #define EXPR(name, fun)\
                case NodeKind::name: {\
                    name##_t* node = reinterpret_cast<name##_t*>(expr);\
                    return fun(node, depth + 1, (args)...);\
                }

            NODEKIND_ENUM(X, SECTION, EXPR, PASS, PASS, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef EXPR

            default:
                return ExprRet();
        }
        // clang-format on
        return ExprRet();
    }

    StmtRet exec(StmtNode_t *stmt, int depth, Args... args) {
        if (!stmt) {
            return StmtRet();
        }
        // trace(depth, "{}", stmt->kind);

        // clang-format off
        switch (stmt->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SECTION(_)
            #define STMT(name, fun)\
                case NodeKind::name: {\
                    name##_t* n = reinterpret_cast<name##_t*>(stmt);\
                    return this->fun(n, depth + 1, (args)...);\
                }

            NODEKIND_ENUM(X, SECTION, PASS, STMT, PASS, PASS)

            #undef X
            #undef PASS
            #undef SECTION
            #undef STMT

            default:
                return StmtRet();
        }
        // clang-format on
        return StmtRet();
    }

#define FUNCTION_GEN(name, fun, rtype)                                           \
    rtype fun(name##_t *node, int depth, Args... args) {                         \
        trace(depth, #name);                                                     \
        return static_cast<Implementation *>(this)->fun(node, depth, (args)...); \
    }

#define X(name, _)
#define SECTION(name)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun, ExprRet)
#define STMT(name, fun)  FUNCTION_GEN(name, fun, StmtRet)
#define MOD(name, fun)   FUNCTION_GEN(name, fun, ModRet)
#define MATCH(name, fun) FUNCTION_GEN(name, fun, PatRet)

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
