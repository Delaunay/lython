#include "Expressions.h"

namespace lython {
namespace AbstractSyntaxTree {
std::size_t pl_hash::operator()(Placeholder &v) const noexcept {
    return _h(*(v.name().get()));
}

Expression::~Expression() {}
UnparsedBlock::~UnparsedBlock() {}
Function::~Function() {}
SeqBlock::~SeqBlock() {}
UnaryOperator::~UnaryOperator() {}
BinaryOperator::~BinaryOperator() {}
Placeholder::~Placeholder() {}


std::ostream &Placeholder::print(std::ostream &out, int32) {
    return out << this->name();
}

std::ostream &UnaryOperator::print(std::ostream &out, int32 indent) {
    this->_expr->print(out, indent);
    return out;
}

std::ostream &BinaryOperator::print(std::ostream &out, int32 indent) {
    this->_lhs->print(out, indent);
    // this->_op
    this->_rhs->print(out, indent);
    return out;
}

std::ostream &UnparsedBlock::print(std::ostream &out, int32 indent) {
    for (auto &tok : _toks)
        tok.print(out, indent);
    return out;
}

std::ostream &Function::print(std::ostream &out, int32 indent) {
    out << "def " << *(_name.get()) << "(";

    for (uint32 i = 0, n = uint32(_args.size()); i < n; ++i) {
        out << *(_args[i].name().get()) << ": " << *(_args[i].type().get());

        if (i < n - 1)
            out << ", ";
    }

    if (_return_type)
        out << "): -> " << *(_return_type.get()) << "\n";
    else
        out << "):\n";

    _body->print(out, indent + 1);
    return out;
}

} // namespace AbstractSyntaxTree
} // namespace lython
