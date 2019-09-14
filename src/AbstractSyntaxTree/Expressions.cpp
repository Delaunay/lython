#include "Expressions.h"
#include "../Logging/logging.h"

#define TRACE_START() trace_start(0, "");
#define TRACE_END() trace_end(0, "");


namespace lython {
namespace AbstractSyntaxTree {
std::size_t pl_hash::operator()(Parameter &v) const noexcept {
    return _h(*(v.name().get()));
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
    for(auto& g: blocks()){
        out << std::string(std::size_t(indent) * 4, ' ');
        g->print(out, indent);
        out << '\n';
    }
    return out;
}

std::ostream &Function::print(std::ostream &out, int32 indent) {
    out << "def " << *(_name.get()) << "(";

    for (uint32 i = 0, n = uint32(_args.size()); i < n; ++i) {
        out << *(_args[i].name().get());

        if (_args[i].type()){
            out << ": ";
            _args[i].type()->print(out, indent);
        }

        if (i < n - 1)
            out << ", ";
    }

    if (_return_type){
        out << ") -> ";
        return_type()->print(out, indent);
        out << ":\n";
    } else
        out << "):\n";

    std::string indentation = std::string(size_t(indent + 1) * 4, ' ');

    if (docstring().size() > 0){
        out << indentation << "\"\"\"" << docstring() << "\"\"\"\n";
    }

    _body->print(out, indent + 1);
    return out;
}

std::ostream &Statement::print(std::ostream &out, int32 indent){
    out << keyword_as_string()[statement()] << " ";
    expr()->print(out, indent);
    return out;
}

std::ostream& Call::print(std::ostream & out, int32 indent){
    function()->print(out, indent);
    out << "(";

    int32 n = int32(arguments().size()) - 1;
    for(int i = 0; i < n; ++i){
        arguments()[std::size_t(i)]->print(out, indent);
        out << ", ";
    }

    if (n >= 0){
         arguments()[std::size_t(n)]->print(out, indent);
    }

    return out << ")";
}

std::ostream &Ref::print(std::ostream &out, int32 indent){
    out << name();
    return out;
}

std::ostream &Struct::print(std::ostream &out, int32 indent){
    out << "struct " << name() << ":\n";
    std::string indentation = std::string((indent + 1) * 4, ' ');

    if (docstring().size() > 0){
        out << indentation << "\"\"\"" << docstring() << "\"\"\"\n";
    }

    for(auto& item : attributes()){
        out << indentation << item.first << ": ";  item.second->print(out, indent);
        out << "\n";
    }
    return out;
}

} // namespace AbstractSyntaxTree
} // namespace lython
