#include "ObjectManager.h"

namespace lython{
ObjectManager::~ObjectManager()
{
    for(int i = 0, n = expression.size(); i < n; i++)
        delete expression[i];

    for(int i = 0, n = prototype.size(); i < n; i++)
        delete prototype[i];

    for(int i = 0, n = function.size(); i < n; i++)
        delete function[i];

#if LLVM_CODEGEN
    /*
    for(int i = 0, n = generated.size(); i < n; i++)
    {
        // llvm::Function* are delete by LLVM
        // but I don't think everything is cleaned up
    }*/
#endif
}

AST::Expression* ObjectManager::new_expr(AST::Expression* expr)
{
    expression.push_back(expr);
    return expr;
}

AST::Prototype* ObjectManager::new_prototype(AST::Prototype* expr)
{
    prototype.push_back(expr);
    return expr;
}

AST::Function* ObjectManager::new_function(AST::Function* expr)
{
    function.push_back(expr);
    return expr;
}

#if LLVM_CODEGEN
llvm::Function* ObjectManager::new_gen_function(llvm::Function* expr)
{
    generated.push_back(expr);
    return expr;
}
#endif

}
