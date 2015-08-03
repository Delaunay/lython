#include "ObjectManager.h"

#include <cassert>

namespace LIBNAMESPACE{

ObjectManager::~ObjectManager()
{
    for(size_t i = 0, n = expression.size(); i < n; i++)
        delete expression[i];

    for(size_t i = 0, n = prototype.size(); i < n; i++)
        delete prototype[i];

    for(size_t i = 0, n = function.size(); i < n; i++)
        delete function[i];

#if LLVM_CODEGEN && 0
    ///*
    for(int i = 0, n = generated.size(); i < n; i++)
    {
        //delete generated[i];
        // llvm::Function* and llvm::Value* are delete by LLVM
        //llvm::Function.eraseFromParent();
        //generated[i]->eraseFromParent();
    }//*/
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
llvm::Value* ObjectManager::new_gen_function(llvm::Value* expr)
{
    generated.push_back(expr);
    return expr;
}
#endif
}
