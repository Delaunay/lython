#include "Expression.h"

#include <cassert>


namespace LIBNAMESPACE{
namespace AbstractSyntaxTree{

Pointer<const string>& none_type()
{
    static Pointer<const string> none(new const string("None"));
    return none;
}

Pointer<const string>& void_type()
{
    static Pointer<const string> void_v(new const string("void"));
    return void_v;
}

Pointer<const string>& double_type()
{
    static Pointer<const string> double_v(new const string("double"));
    return double_v;
}

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
{
    str << "Empty Expression";
}

VariableExpression::VariableExpression(const string &name):
    name(name), Expression(Type_VariableExpression)
{}

void VariableExpression::print(ostream& str, int i)
{
    str << I(i) << "v[" << name << "]";
}

BinaryExpression::BinaryExpression(string op, Expression *lhs, Expression *rhs):
    op(op), lhs(lhs), rhs(rhs), Expression(Type_BinaryExpression)
{}

void BinaryExpression::print(ostream& str, int i)
{
    str << I(i) << "bin'" << op << "'(";

        lhs->print(str);

        str << ", ";

        rhs->print(str);

    str << ")";
}

Function::Function(Prototype *proto, Expression *body):
    prototype(proto), body(body), Expression(Type_Function)
{}

void Function::print(ostream& str, int i)
{
    prototype->print(str, i);

    body->print(str, i + 1);
    str << "\n";
}


Prototype::Prototype(const string &name, const Arguments& args,
                     bool isoperator, unsigned prec):
    name(name), args(args), _is_operator(isoperator), _precedence(prec),
    return_type_t(void_type())
{}


bool Prototype::is_unary() const {   return _is_operator && args.size() == 1; }
bool Prototype::is_binary()const {   return _is_operator && args.size() == 2; }
const bool& Prototype::is_operator() const { return _is_operator; }

const unsigned& Prototype::precedence() const {   return _precedence; }

string Prototype::operator_name() const
{
    assert(is_unary() || is_binary());

    return name;//[name.size() - 1];
}

void Prototype::print(ostream& str, int i)
{
    str << I(i) << "fn " << name << " (";

    for (size_t i = 0, n = args.size(); i < n; i++)
    {
//        str << args[i].first;
        str << args[i];

        if (i != n - 1)
            str << ", ";
    }

    str << ")\n";
}

CallExpression::CallExpression(const std::string &callee, Arguments& args, char c):
    callee(callee), args(args), Expression(Type_CallExpression), ptype(c)
{}

void CallExpression::print(ostream& str, int i)
{
    str << I(i) << callee << ptype;

    for (size_t i = 0, n = args.size(); i < n; i++)
    {
        args[i]->print(str);

        if (i != n - 1)
            str << ", ";
    }

    if (ptype == '(')
        str << ")";
    else
        str << "]";
}

IfExpression::IfExpression(Expression* cond, Expression* then, Expression* lse):
    cond(cond), then(then), els(lse), Expression(Type_IfExpression, true)
{}

void IfExpression::print(ostream& str, int i)
{
    str << I(i) << "if ";

    cond->print(str, 0);

    str << ":\n";

    then->print(str, i + 1);

    str << "\n"
    << I(i) << "else: \n";

    els->print(str, i + 1);
}


ForExpression::ForExpression(const std::string &var,
              AST::Expression* s, AST::Expression* e ,
              AST::Expression* st, AST::Expression* bd):
    var(var), start(s), end(e), step(st), body(bd),
    Expression(Type_ForExpression, true)
{}


ForExpression::ForExpression(const std::string &var, AST::Expression*s, AST::Expression* bd):
    var(var), start(s), end(0), step(0), body(bd),
    Expression(Type_ForExpression, true)
{}


void ForExpression::print(ostream& str, int i)
{
    if (end != 0 && step != 0)
    {
        str << I(i) << "for(" << var << ": ";
            start->print(str);

        str << " ; ";
            end->print(str);

        str << " ; ";
            step->print(str);

        str << ")\n";
            body->print(str, i + 1);
    }
    else
    {
        str << I(i) << "for " << var << " in "; start->print(str); str << ":\n";
            body->print(str, i + 1);
    }
}

UnaryExpression::UnaryExpression(string opcode, Expression *operand):
    opcode(opcode), operand(operand),
    Expression(Type_UnaryExpression)
{}

void UnaryExpression::print(ostream& str, int i)
{
    str << I(i) << "Un'" << opcode << "'(";

    operand->print(str);

    str << ")";
}

void ClassExpression::print(ostream& str, int idt)
{
    str << I(idt) << "class " + (*type) + ":\n";

    for(auto i = attributes.begin(); i != attributes.end(); ++i)
    {
        (*i).second->print(str, idt + 1);
        str << "\n";
    }

    for(auto i = methods.begin(); i != methods.end(); ++i)
    {
        (*i).second->print(str, idt + 1);
        //str << "\n";
    }
}


}
}
