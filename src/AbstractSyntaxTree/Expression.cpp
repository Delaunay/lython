#include "Expression.h"

namespace lython{
namespace AbstractSyntaxTree{

string I(unsigned int n)
{
    string str = "";

    for (unsigned int i = 0; i < n; i++)
        str += "    ";

    return str;
}

Expression::~Expression()
{}

void Expression::print(ostream& str, int i)
{}


VariableExpression::VariableExpression(const string &name):
    name(name)
{}

void VariableExpression::print(ostream& str, int i)
{
    str << I(i) << name;
}

BinaryExpression::BinaryExpression(char op, Expression *lhs, Expression *rhs):
    op(op), lhs(lhs), rhs(rhs)
{}

void BinaryExpression::print(ostream& str, int i)
{
    str << I(i) << "'" << op << "'(";

        lhs->print(str);

        str << ", ";

        rhs->print(str);

    str << ")";
}

Function::Function(Prototype *proto, Expression *body):
    prototype(proto), body(body)
{}

void Function::print(ostream& str, int i)
{
    str << I(i);
    prototype->print(str, i);

    str << I(i + 1);

    body->print(str, i + 1);
    str << "\n\n";
}


Prototype::Prototype(const string &name, const Arguments& args):
    name(name), args(args)
{}

void Prototype::print(ostream& str, int i)
{
    str << I(i) << "fn " << name << " (";

    for (int i = 0, n = args.size(); i < n; i++)
    {
        str << args[i];

        if (i != n - 1)
            str << ", ";
    }

    str << ")\n";
}

CallExpression::CallExpression(const std::string &callee, Arguments& args):
    callee(callee), args(args)
{}

void CallExpression::print(ostream& str, int i)
{
    str << I(i) << callee << "(";

    for (int i = 0, n = args.size(); i < n; i++)
    {
        args[i]->print(str);

        if (i != n - 1)
            str << ", ";
    }

    str << ")";
}

IfExpression::IfExpression(Expression* cond, Expression* then, Expression* lse):
    cond(cond), then(then), els(lse)
{}

void IfExpression::print(ostream& str, int i)
{
    str << "if ";

    cond->print(str, 0);

    str << ":\n" << I(i + 1);

    then->print(str, i);

    str << "\n"
    << I(i) << "else: \n";

    els->print(str, i + 1);

}
ForExpression::ForExpression(const std::string &var,
              AST::Expression* s, AST::Expression* e ,
              AST::Expression* st, AST::Expression* bd):
    var(var), start(s), end(e), step(st), body(bd)
{}

}
}
