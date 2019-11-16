#include "expressions.h"
#include "../logging/logging.h"
#include "../parser/module.h"

#define TRACE_START() trace_start(0, "");
#define TRACE_END() trace_end(0, "");

namespace lython {
namespace AbstractSyntaxTree {
std::size_t pl_hash::operator()(Parameter &v) const noexcept {
    auto n = v.name();
    String tmp(std::begin(n), std::end(n));
    return _h(tmp);
}

Expression::~Expression() {}
UnparsedBlock::~UnparsedBlock() {}
Function::~Function() {}
SeqBlock::~SeqBlock() {}
UnaryOperator::~UnaryOperator() {}
BinaryOperator::~BinaryOperator() {}
Parameter::~Parameter() {}
Call::~Call() {}

// FunctionPrototype::~FunctionPrototype(){}

std::ostream &Parameter::print(std::ostream &out, int32) {
    return out << this->name();
}

std::ostream &UnaryOperator::print(std::ostream &out, int32 indent) {
    // debug("UnaryPrint");
    this->_expr->print(out, indent);
    return out;
}

std::ostream &BinaryOperator::print(std::ostream &out, int32 indent) {
    this->_lhs->print(out, indent);
    out << ' ';
    this->_op->print(out, indent);
    out << ' ';
    this->_rhs->print(out, indent);
    return out;
}

std::ostream &UnparsedBlock::print(std::ostream &out, int32 indent) {
    for (auto &tok : _toks)
        tok.print(out, indent);
    return out;
}

std::ostream &SeqBlock::print(std::ostream &out, int32 indent) {
    for (auto &g : blocks()) {
        out << std::string(std::size_t(indent) * 4, ' ');
        g->print(out, indent);
    }
    return out;
}

std::ostream &Function::print(std::ostream &out, int32 indent) {
    // debug("Function Print");

    out << "def " << _name << "(";

    for (uint32 i = 0, n = uint32(_args.size()); i < n; ++i) {
        out << _args[i].name();

        if (_args[i].type()) {
            out << ": ";
            _args[i].type()->print(out, indent);
        }

        if (i < n - 1)
            out << ", ";
    }

    if (_return_type) {
        out << ") -> ";
        return_type()->print(out, indent);
        out << ":\n";
    } else
        out << "):\n";

    std::string indentation = std::string(size_t(indent + 1) * 4, ' ');

    if (docstring().size() > 0) {
        out << indentation << "\"\"\"" << docstring() << "\"\"\"\n";
    }

    _body->print(out, indent + 1);
    return out;
}

std::ostream &Statement::print(std::ostream &out, int32 indent) {
    out << keyword_as_string()[statement()] << " ";
    expr()->print(out, indent);
    return out;
}

std::ostream &Call::print(std::ostream &out, int32 indent) {
    function()->print(out, indent);
    out << "(";

    int32 n = int32(arguments().size()) - 1;
    for (int i = 0; i < n; ++i) {
        arguments()[std::size_t(i)]->print(out, indent);
        out << ", ";
    }

    if (n >= 0) {
        arguments()[std::size_t(n)]->print(out, indent);
    }

    return out << ")";
}

std::ostream &Ref::print(std::ostream &out, int32) {
    out << name();
    return out;
}

std::ostream &Struct::print(std::ostream &out, int32 indent) {
    out << "struct " << name() << ":\n";
    std::string indentation = std::string(std::size_t((indent + 1) * 4), ' ');

    if (docstring().size() > 0) {
        out << indentation << "\"\"\"" << docstring() << "\"\"\"\n";
    }

    for (auto &item : attributes()) {
        out << indentation << item.first << ": ";
        item.second->print(out, indent);
        out << "\n";
    }
    return out;
}

String ReversePolishExpression::to_infix(Stack<MathNode>::Iterator &iter,
                                              int prev) {
    int pred;
    MathNode op = *iter;
    iter++;

    switch (op.kind) {
    case MathKind::Operator: {
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

    case MathKind::Function: {
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
            op.ref->print(ss);
            fun = ss.str();
        }
        return fun + '(' + str_args + ')';
    }

    case MathKind::VarRef: {
        String var = op.name;
        if (op.ref){
            std::basic_stringstream<char, std::char_traits<char>, Allocator<char>> ss;
            op.ref->print(ss);
            var = ss.str();
        }
        return var;
    }
    case MathKind::Value: {
        return op.name;
    }

    case MathKind::None:
        return "<None>";
    }

    return "<Error>";
}

std::ostream &Builtin::print(std::ostream &out, int32) {
    out << name;
    // out << "Builtin(" << name << ", ";
    // type->print(out, indent);
    // out << ")";
    return out;
}

std::ostream &Type::print(std::ostream &out, int32) {
    return out << name;
}

std::ostream &ReversePolishExpression::print(std::ostream &out, int32) {
    auto iter = std::begin(stack);
    return out << to_infix(iter);
}

std::ostream &Value::print(std::ostream &out, int32 indent) {
    switch(tag){
    case CTInt: return out << v_int;
    case CTDouble: return out << v_double;
    }

    return out;
}


} // namespace AbstractSyntaxTree
} // namespace lython
