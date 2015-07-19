#include "ObjectManager.h"

#include <cassert>

namespace lython{
ObjectManager::~ObjectManager()
{
    for(size_t i = 0, n = expression.size(); i < n; i++)
    {
        bool b = safe_delete(expression[i]);

        //printf("\n\n %i \n", b);

        assert( b && "Critical Failure: Could Not delete an Expression");

    }

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

bool safe_delete(AST::Expression* exp)
{
    switch(exp->type)
    {
    case AST::Type_Expression:
        delete (AST::Expression*)(exp);
        return true;

    case AST::Type_BinaryExpression:
        delete (AST::BinaryExpression*)(exp);
        return true;

    case AST::Type_CallExpression:
        delete (AST::CallExpression*)(exp);
        return true;

    case AST::Type_ForExpression:
        delete (AST::ForExpression*)(exp);
        return true;

    case AST::Type_Function:
        delete (AST::Function*)(exp);
        return true;

    case AST::Type_IfExpression:
        delete (AST::IfExpression*)(exp);
        return true;

    case AST::Type_MultilineExpression:
        delete (AST::MultilineExpression*)(exp);
        return true;

    case AST::Type_MutableVariableExpression:
        delete (AST::MutableVariableExpression*)(exp);
        return true;

    case AST::Type_Prototype:
        delete (AST::Prototype*)(exp);
        return true;

    case AST::Type_TypedExpression:
        delete (AST::TypedExpression<>*)(exp);
        return true;

    case AST::Type_UnaryExpression:
        delete (AST::UnaryExpression*)(exp);
        return true;

    case AST::Type_VariableExpression:
        delete (AST::VariableExpression*)(exp);
        return true;

    default:
        return false;
    }
}
}
