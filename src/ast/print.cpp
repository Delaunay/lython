#include "print.h"
#include "../parser/module.h"
#include "visitor.h"

namespace lython {

String to_infix(Stack<AST::MathNode>::Iterator &iter, int prev=0);

struct ASTPrinter: public Visitor<ASTPrinter, std::ostream&>{
    std::ostream& out;
    int indent;

    ASTPrinter(std::ostream& out, int indent = 0):
        out(out), indent(indent)
    {}

    std::ostream& undefined(AST::Node*, std::size_t){
        return out << "<undefined>";
    }

    std::ostream &parameter(AST::Parameter* param, std::size_t) {
        return out << param->name;
    }

    std::ostream &unary(AST::UnaryOperator* un, size_t d) {
        visit(un, d);
        return out;
    }

    std::ostream &binary(AST::BinaryOperator* bin, std::size_t d) {
        visit(bin->lhs, d);
        out << ' ';
        visit(bin->op, d);
        out << ' ';
        visit(bin->rhs, d);
        return out;
    }

    std::ostream &sequential(AST::SeqBlock* blocks, std::size_t d) {
        for (auto &g : blocks->blocks) {
            out << std::string(std::size_t(indent) * 4, ' ');
            visit(g, d);
        }
        return out;
    }

    std::ostream &unparsed(AST::UnparsedBlock* blocks, std::size_t) {
        for (auto &tok : blocks->tokens)
            tok.print(out, indent);
        return out;
    }

    std::ostream &reverse_polish(AST::ReversePolish* rev, std::size_t) {
        auto iter = std::begin(rev->stack);
        return out << to_infix(iter);
    }

    std::ostream &statement(AST::Statement* stmt, std::size_t d) {
        out << keyword_as_string()[stmt->statement] << " ";
        visit(stmt->expr, d);
        return out;
    }

    std::ostream &reference(AST::Reference* ref, std::size_t) {
        return out << ref->name;
    }

    std::ostream &builtin(AST::Builtin* blt, int32) {
        return out << blt->name;
    }

    std::ostream &type(AST::Type* type, std::size_t) {
        return out << type->name;
    }

    std::ostream &value(AST::Value* val, std::size_t d) {
        val->value.print(out);
        if (val->type){
            out << ": ";
            visit(val->type, d);
        }
        return out;
    }

    std::ostream &struct_type(AST::Struct* cstruct, std::size_t d) {
        out << "struct " << cstruct->name << ":\n";
        std::string indentation = std::string(std::size_t((indent + 1) * 4), ' ');

        if (cstruct->docstring.size() > 0) {
            out << indentation << "\"\"\"" << cstruct->docstring << "\"\"\"\n";
        }

        for (auto &item : cstruct->attributes) {
            out << indentation << item.first << ": ";
            visit(item.second, d);
            out << "\n";
        }
        return out;
    }

    std::ostream &call(AST::Call* call, std::size_t d) {
        call->function.print(out, indent);
        out << "(";

        int32 n = int32(call->arguments.size()) - 1;
        for (int i = 0; i < n; ++i) {
            visit(call->arguments[std::size_t(i)], d);
            out << ", ";
        }

        if (n >= 0) {
            visit(call->arguments[std::size_t(n)], d);
        }

        return out << ")";
    }

    std::ostream &arrow(AST::Arrow* aw, std::size_t d) {
        int n = int(aw->params.size()) - 1;

        out << "(";

        for (int i = 0; i < n; ++i) {
            visit(&aw->params[size_t(i)], d);
            out << ", ";
        }

        if (n >= 0) {
            visit(&aw->params[size_t(n)], d);
        }

        out << ") -> ";
        out << aw->return_type;

        aw->return_type.print(out, indent);
        return out;
    }

    std::ostream &function(AST::Function* fun, std::size_t d) {
        // debug("Function Print");

        out << "def " << fun->name << "(";

        for (uint32 i = 0, n = uint32(fun->args.size()); i < n; ++i) {
            out << fun->args[i].name;

            if (fun->args[i].type) {
                out << ": ";
                visit(fun->args[i].type, d);
            }

            if (i < n - 1)
                out << ", ";
        }

        if (fun->return_type) {
            out << ") -> ";
            visit(fun->return_type, d);
            out << ":\n";
        } else
            out << "):\n";

        std::string indentation = std::string(size_t(indent + 1) * 4, ' ');

        if (fun->docstring.size() > 0) {
            out << indentation << "\"\"\"" << fun->docstring << "\"\"\"\n";
        }

        indent += 1;
        visit(fun->body, d);
        indent -= 1;
        return out << "\n";
    }
};

std::ostream& print(std::ostream& out, const Expression expr, int indent){
    return ASTPrinter(out, indent).visit(expr);
}


String to_infix(Stack<AST::MathNode>::Iterator &iter, int prev) {
    int pred;
    AST::MathNode op = *iter;
    iter++;

    switch (op.kind) {
    case AST::MathKind::Operator: {
        std::tie(pred, std::ignore) = Module::default_precedence()[op.name];

        auto rhs = to_infix(iter, pred);
        auto lhs = to_infix(iter, pred);

        auto expr = lhs + ' ' + op.name + ' ' + rhs;
        // if parent has lower priority we have to put parens
        // if the priority is the same we still put parens for explicitness
        // but we do not have to
        if (prev >= pred)
            return '(' + expr + ')';

        return expr;
    }

    case AST::MathKind::Function: {
        int nargs = op.arg_count;

        std::vector<String> args;
        for (int i = 0; i < nargs - 1; ++i) {
            args.push_back(to_infix(iter));
            args.emplace_back(", ");
        }
        if (nargs > 0) {
            args.push_back(to_infix(iter));
        }

        // arguments are in reverse
        String str_args =
            std::accumulate(std::rbegin(args), std::rend(args), String(),
                            [](String acc, String &b) { return acc + b; });


        String fun = op.name;
        if (op.ref){
            std::basic_stringstream<char, std::char_traits<char>, Allocator<char>> ss;
            op.ref.print(ss);
            fun = ss.str();
        }
        return fun + '(' + str_args + ')';
    }

    case AST::MathKind::VarRef: {
        String var = op.name;
        if (op.ref){
            std::basic_stringstream<char, std::char_traits<char>, Allocator<char>> ss;
            op.ref.print(ss);
            var = ss.str();
        }
        return var;
    }
    case  AST::MathKind::Value: {
        return op.name;
    }

    case AST::MathKind::None:
        return "<None>";
    }

    return "<Error>";
}
}
