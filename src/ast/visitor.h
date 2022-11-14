#ifndef LYTHON_AST_VISITOR_HEADER
#define LYTHON_AST_VISITOR_HEADER

#include "ast/nodes.h"
#include "dependencies/coz_wrap.h"
#include "logging/logging.h"

#ifndef LY_MAX_VISITOR_RECURSION_DEPTH
#    define LY_MAX_VISITOR_RECURSION_DEPTH 256
#endif

namespace lython {

NEW_EXCEPTION(NullPointerError)

struct DefaultVisitorTrait {
    using Trace               = std::true_type;
    using LimitRecursionDepth = std::true_type;
    using StmtRet             = StmtNode*;
    using ExprRet             = ExprNode*;
    using ModRet              = ModNode*;
    using PatRet              = Pattern*;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

#ifdef __linux__
#    define LYTHON_INLINE
#else
#    define LYTHON_INLINE __forceinline
#endif

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
    using Trace   = typename VisitorTrait::Trace;
    using StmtRet = typename VisitorTrait::StmtRet;
    using ExprRet = typename VisitorTrait::ExprRet;
    using ModRet  = typename VisitorTrait::ModRet;
    using PatRet  = typename VisitorTrait::PatRet;

#define SELECT_TYPE(T) typename std::conditional<isConst, T const, T>::type;

    using Node_t     = SELECT_TYPE(Node);
    using ModNode_t  = SELECT_TYPE(ModNode);
    using Pattern_t  = SELECT_TYPE(Pattern);
    using ExprNode_t = SELECT_TYPE(ExprNode);
    using StmtNode_t = SELECT_TYPE(StmtNode);

#undef SELECT_TYPE

#define TYPE_GEN(rtype) \
    using rtype##_t = typename std::conditional<isConst, rtype const, rtype>::type;

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, fun)  TYPE_GEN(name)
#define STMT(name, fun)  TYPE_GEN(name)
#define MOD(name, fun)   TYPE_GEN(name)
#define MATCH(name, fun) TYPE_GEN(name)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef TYPE_GEN

    template <typename U, typename T>
    Array<U> exec(Array<T>& body, int depth, Args... args) {
        int      k = 0;
        Array<U> types;
        for (auto& stmt: body) {
            types.push_back(exec(stmt, depth, (args)...));
            k += 1;
        }
        return types;
    };

    template <typename U, typename T>
    Optional<U> exec(Optional<T>& maybe, int depth, Args... args) {
        if (maybe.has_value()) {
            return some<U>(exec(maybe.value(), depth, (args)...));
        }
        return none<U>();
    };

    template <typename T>
    T exec(Node_t* n, int depth, Args... args) {
        switch (n->family()) {
        case NodeFamily::Module: return exec(reinterpret_cast<ModNode_t*>(n), depth, (args)...);
        case NodeFamily::Statement: return exec(reinterpret_cast<StmtNode_t*>(n), depth, (args)...);
        case NodeFamily::Expression:
            return exec(reinterpret_cast<ExprNode_t*>(n), depth, (args)...);
        case NodeFamily::Pattern: return exec(reinterpret_cast<Pattern_t*>(n), depth, (args)...);
        }
        return T();
    }

    ModRet exec(ModNode_t* mod, int depth, Args... args) {
        // clang-format off
        // trace(depth, "{}", mod->kind);

        check_depth(depth);

        switch (mod->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SSECTION(_)
            #define MOD(name, fun)\
                case NodeKind::name: {\
                    name##_t* m = reinterpret_cast<name##_t*>(mod);\
                    return fun(m, depth + 1, (args)...);\
                }

            NODEKIND_ENUM(X, SSECTION, PASS, PASS, MOD, PASS)

            #undef X
            #undef PASS
            #undef SSECTION
            #undef MOD

            default:
                return ModRet();

        }
        // clang-format on
        return ModRet();
    }

    PatRet exec(Pattern_t* pat, int depth, Args... args) {
        if (!pat) {
            return PatRet();
        }

        check_depth(depth);

        // trace(depth, "{}", pat->kind);
        // clang-format off
        switch (pat->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SSECTION(_)
            #define MATCH(name, fun)\
                case NodeKind::name: {\
                    name##_t* p = reinterpret_cast<name##_t*>(pat);\
                    return fun(p, depth + 1, (args)...);\
                }

            NODEKIND_ENUM(X, SSECTION, PASS, PASS, PASS, MATCH)

            #undef X
            #undef PASS
            #undef SSECTION
            #undef MATCH

            default:
                return PatRet();
        }
        // clang-format on
        return PatRet();
    }

    ExprRet exec(ExprNode_t* expr, int depth, Args... args) {
        if (!expr) {
            return ExprRet();
        }

        check_depth(depth);

        // trace(depth, "{}", expr->kind);
        // clang-format off
        switch (expr->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SSECTION(_)
            #define EXPR(name, fun)\
                case NodeKind::name: {\
                    name##_t* node = reinterpret_cast<name##_t*>(expr);\
                    return fun(node, depth + 1, (args)...);\
                }

            NODEKIND_ENUM(X, SSECTION, EXPR, PASS, PASS, PASS)

            #undef X
            #undef PASS
            #undef SSECTION
            #undef EXPR

            default:
                return ExprRet();
        }
        // clang-format on
        return ExprRet();
    }

    void check_depth(int depth) {
        if (VisitorTrait::MaxRecursionDepth > 0 && depth > VisitorTrait::MaxRecursionDepth) {
            throw std::runtime_error("");
        }
    }

    StmtRet exec(StmtNode_t* stmt, int depth, Args... args) {
        if (!stmt) {
            debug("Null statement");
            return StmtRet();
        }

        check_depth(depth);

        // clang-format off
        switch (stmt->kind) {

            #define X(name, _)
            #define PASS(a, b)
            #define SSECTION(_)
            #define STMT(name, fun)\
                case NodeKind::name: {\
                    name##_t* n = reinterpret_cast<name##_t*>(stmt);\
                    return this->fun(n, depth + 1, (args)...);\
                }

            NODEKIND_ENUM(X, SSECTION, PASS, STMT, PASS, PASS)

            #undef X
            #undef PASS
            #undef SSECTION
            #undef STMT

            default:
                return StmtRet();
        }
        // clang-format on
        return StmtRet();
    }

#define FUNCTION_GEN(name, fun, rtype)                                          \
    LYTHON_INLINE rtype fun(name##_t* node, int depth, Args... args) {          \
        if (Trace::value) {                                                     \
            trace(depth, #name);                                                \
        }                                                                       \
        return static_cast<Implementation*>(this)->fun(node, depth, (args)...); \
    }

#define X(name, _)
#define SSECTION(name)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun, ExprRet)
#define STMT(name, fun)  FUNCTION_GEN(name, fun, StmtRet)
#define MOD(name, fun)   FUNCTION_GEN(name, fun, ModRet)
#define MATCH(name, fun) FUNCTION_GEN(name, fun, PatRet)

    NODEKIND_ENUM(X, SSECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SSECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef FUNCTION_GEN

};  // namespace lython

}  // namespace lython
#endif
