#include "fold.h"

namespace lython {

struct Traverse{
    std::function<void(Expression)> fun;

    Traverse(std::function<void(Expression)> fun):
        fun(fun)
    {}

    void foreach(Expression node){
        switch (node.kind()) {
        // leaves
        case AST::NodeKind::KType:
        case AST::NodeKind::KValue:
            return fun(node);
        // branches
        }
    }
};


void foreach(std::function<void(Expression)> fun, Expression node){
    Traverse(fun).foreach(node);
}
}
