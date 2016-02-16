#include "Expression.h"

namespace LIBNAMESPACE
{

Scope::~Scope()
{
    if (_owner)
        for(auto i:_Scope)
            delete i.second;
}

void Scope::print(std::ostream& out, AST::Expression* e)
{
    AST::ExpressionType t = e->etype;

    switch(t)
    {
        case AST::Type_VariableExpression:
            out << "Variable";
            return;

        case AST::Type_ClassExpression:
            out << "Class Declaration";
            return;

        case AST::Type_Function:
            out << "Function Declaration";
            return;

        case AST::Type_BinaryExpression:{
            AST::BinaryExpression* b = (AST::BinaryExpression*) e;

            if (b->lhs->etype == AST::Type_VariableExpression)
                out << "Variable";

            if (b->rhs->etype == AST::Type_Expression){
                out << " ";
                b->rhs->print(out);
            }
            return;
    }

        default:
            out  << t;
            return;
    }
}

void Scope::print(std::ostream& out)
{
    out << "Loaded Object in '" << _name << "' Scope: {\n";

    for(auto& i:_Scope)
    {
        out << "    " << i.first << " : ";

        Scope::print(out, i.second);

        out << "\n";
    }

    out << "}\n";

}

}
