#ifndef LYTHON_AST_VISITOR_HEADER
#define LYTHON_AST_VISITOR_HEADER

#include "expressions.h"
#include "nodes.h"

namespace lython {

/*!
 * Visitor implemented using static polymorphism.
 * This implementation has 2 advantages:
 *  1. Enable better inlining & devirtualization than vtables
 *      - More cache friendly
 *  2. Reduce the number of indirection from 2 to 1
 *      - 2 because of the double vtable lookup (visitor vtable + node vtable)
 */
template<typename Implementation, typename ReturnType, typename ... Args>
struct Visitor{
    ReturnType visit(Expression expr, std::size_t depth=0, Args... args){
        return visit(expr.ref<AST::Node>(), depth, args...);
    }

    ReturnType visit(AST::Node* expr, std::size_t depth=0, Args... args){
        switch (expr->kind){
        #define KIND(name, funname)\
            case AST::NodeKind::K##name: {\
                auto ref = static_cast<AST::name*>(expr);\
                return funname(ref, depth + 1, args...);\
            }
            NODE_KIND_ENUM(KIND)
        #undef KIND
        default:
            return undefined(expr, depth + 1, args...);
        }
    }

    ReturnType undefined(AST::Node* node, std::size_t depth, Args... args){
        return static_cast<Implementation*>(this)->undefined(node, depth, args...);
    }

    #define KIND(name, funname)\
        ReturnType funname(AST::name* node, std::size_t depth, Args... args){\
            return static_cast<Implementation*>(this)->funname(node, depth, args...);\
        }
        NODE_KIND_ENUM(KIND)
    #undef KIND

    // Implementation visitor;
};

}
#endif
