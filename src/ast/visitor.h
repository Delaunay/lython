#ifndef LYTHON_AST_VISITOR_HEADER
#define LYTHON_AST_VISITOR_HEADER

#include "ast/expressions.h"
#include "ast/nodes.h"

namespace lython {

NEW_EXCEPTION(NullPointerError)

#ifdef __linux__
#define KIWI_INLINE __attribute__((always_inline))
#else
#define KIWI_INLINE __forceinline
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
template<typename Implementation, typename Expr, typename ReturnType, typename ... Args>
struct BaseVisitor{
    // Generate alias to Node Reference depending on if Expr is const or not
    using Expr_t = Expr;
    using Node_t = typename std::conditional<std::is_const<Expr>::value, const AST::Node*, AST::Node*>::type;

    #define KIND(name, funname)\
        using name##_t = typename std::conditional<std::is_const<Expr>::value, const AST::name*, AST::name*>::type;
        NODE_KIND_ENUM(KIND)
    #undef KIND

    // force inline to make the stacktrace prettier
    KIWI_INLINE ReturnType visit(Expr_t expr, std::size_t depth=0, Args... args)  {
        using Node_T = typename std::conditional<std::is_const<Expr>::value, const AST::Node, AST::Node>::type;

        Node_t node = expr.template ref<Node_T>();
        return visit(node, depth, args...);
    }

    // Dispatch table
    ReturnType visit(Node_t expr, std::size_t depth=0, Args... args){
        switch (expr->kind){
        #define KIND(name, funname)\
            case AST::NodeKind::K##name: {\
                auto ref = reinterpret_cast<name##_t>(expr);\
                return funname(ref, depth + 1, args...);\
            }
            NODE_KIND_ENUM(KIND)
        #undef KIND
        default:
            return undefined(expr, depth + 1, args...);
        }
    }

    ReturnType undefined(Node_t node, std::size_t depth, Args... args){
        return static_cast<Implementation*>(this)->undefined(node, depth, args...);
    }

    // inline will make the stacktrace better and throw an error if the function is not implemented
    #define KIND(name, funname)\
        KIWI_INLINE ReturnType funname(name##_t node, std::size_t depth, Args... args){\
            return reinterpret_cast<Implementation*>(this)->funname(node, depth, args...);\
        }
        NODE_KIND_ENUM(KIND)
    #undef KIND
};

// Const vistor means the Expression are not modified
// the state of the visitor is still mutable
template<typename Implementation, typename ReturnType, typename ... Args>
using ConstVisitor = BaseVisitor<Implementation, const Expression, ReturnType, Args...>;

template<typename Implementation, typename ReturnType, typename ... Args>
using Visitor = BaseVisitor<Implementation, Expression, ReturnType, Args...>;


}
#endif
