#ifndef LYTHON_AST_VISITOR_HEADER
#define LYTHON_AST_VISITOR_HEADER

#include "ast/expressions.h"
#include "ast/nodes.h"

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
template <typename Implementation, typename Expr, typename ReturnType, typename... Args>
struct BaseVisitor {
    // Generate alias to Node Reference depending on if Expr is const or not
    using Expr_t = Expr;
    using Node_t =
        typename std::conditional<std::is_const<Expr>::value, const Node *, Node *>::type;

#define TYPEGEN(name) \
    using name##_t =  \
        typename std::conditional<std::is_const<Expr>::value, const name *, name *>::type;
    NODE_KIND_ENUM(KIND)

#define X(name, _)
#define SECTION(name)
#define EXPR(name, _)  TYPEGEN(name)
#define STMT(name, _)  TYPEGEN(name)
#define MOD(name, _)   TYPEGEN(name)
#define MATCH(name, _) TYPEGEN(name)

    NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef TYPEGEN

    // force inline to make the stacktrace prettier
    KIWI_INLINE ReturnType visit(Expr_t expr, std::size_t depth = 0, Args... args) {
        Node_t node = expr.template ref<Node_t>();
        return visit(node, depth, args...);
    }

    // Dispatch table
    ReturnType visit(Node_t expr, std::size_t depth = 0, Args... args) {
        switch (expr->kind) {

#define CASEGEN(name, funname)                   \
case NodeKind::name: {                           \
    auto ref = reinterpret_cast<name##_t>(expr); \
    return funname(ref, depth + 1, args...);     \
}

#define X(name, _)
#define SECTION(name)
#define EXPR(name, funname)  CASEGEN(name, funname)
#define STMT(name, funname)  CASEGEN(name, funname)
#define MOD(name, funname)   CASEGEN(name, funname)
#define MATCH(name, funname) CASEGEN(name, funname)

            NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef CASEGEN

        default:
            return undefined(expr, depth + 1, args...);
        }
    }

    ReturnType undefined(Node_t node, std::size_t depth, Args... args) {
        return static_cast<Implementation *>(this)->undefined(node, depth, args...);
    }

// inline will make the stacktrace better and throw an error if the function is not implemented
#define KIND(name, funname)                                                             \
    KIWI_INLINE ReturnType funname(name##_t node, std::size_t depth, Args... args) {    \
        return reinterpret_cast<Implementation *>(this)->funname(node, depth, args...); \
    }
    NODE_KIND_ENUM(KIND)
#undef KIND
};

// Const vistor means the Expression are not modified
// the state of the visitor is still mutable
template <typename Implementation, typename ReturnType, typename... Args>
using ConstVisitor = BaseVisitor<Implementation, const Expression, ReturnType, Args...>;

template <typename Implementation, typename ReturnType, typename... Args>
using Visitor = BaseVisitor<Implementation, Expression, ReturnType, Args...>;

} // namespace lython
#endif
