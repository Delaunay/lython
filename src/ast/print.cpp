#include "sexpression.h"
#include "utilities/strings.h"
#include "magic.h"

namespace lython {

String Pattern::__str__() const {
    StringStream ss;
    print(ss);
    return ss.str();
}

String Node::__str__() const {
    StringStream ss;
    print(ss, 0);
    return ss.str();
}

void ConstantValue::print(std::ostream& out) const {
    switch (kind) {
        case TString: 
            out << "\"" << value.string << "\"";

        case TFloat: 
            out << value.decimal;

        case TInt: 
            out << value.integer;
    }
}

void Slice::print(std::ostream& out, int indent) const {
    if (lower.has_value()) {
        lower.value()->print(out, indent);
    }

    out << ":";

    if (upper.has_value()) {
        upper.value()->print(out, indent);
    }

    if (step.has_value()) {
        out << ":";
        step.value()->print(out, indent);
    }
}

void print(std::ostream& out, int indent, Array<StmtNode*> const& body) {
    for(auto& stmt: body) {
        out << std::string(indent * 4, ' ');
        stmt->print(out, indent);
    }
}

void ExceptHandler::print(std::ostream& out, int indent) const {
    out << "except ";

    if (type.has_value()) {
        type.value()->print(out, indent);
    }

    if (name.has_value()) {
        out << name.value();
    }
    
    out << ":\n";
    lython::print(out, indent + 1, body);
}

void TupleExpr::print(std::ostream& out, int indent) const {
    out << "(" << join(", ", elts) << ")";
}

void ListExpr::print(std::ostream& out, int indent) const {
    out << "[" << join(", ", elts) << "]";
}

void SetExpr::print(std::ostream& out, int indent) const {
    out << "{" << join(", ", elts) << "}";
}

void DictExpr::print(std::ostream& out, int indent) const {
    Array<String> strs;
    strs.reserve(keys.size());

    for(int i = 0; i < keys.size(); i++) {
        // FIXME std::string -> String conversion
        strs.push_back(String(fmt::format("{}: {}", str(keys[i]), str(values[i])).c_str()));
    }

    out << "{" << join(", ", strs) << "}";
}

void Comprehension::print(std::ostream& out, int indent) const {
    out << "for ";
    target->print(out, indent);
    out << " in ";
    iter->print(out, indent);

    for(auto expr: ifs) {
        out << "if ";
        expr->print(out, indent);
    }
}

void Keyword::print(std::ostream& out, int indent) const {
    if (arg.has_value()) {
        out << arg.value();
    }

    if (value != nullptr) {
        out << " = ";
        value->print(out, indent);
    }
}

void Alias::print(std::ostream& out, int indent) const {
    out << name;
    if (asname.has_value()) {
        out << " as " << asname.value();
    }
}

void WithItem::print(std::ostream& out, int indent) const {
    context_expr->print(out, indent);
    if (optional_vars.has_value()){
        out << " as ";
        optional_vars.value()->print(out, indent);
    }
}

void MatchValue::print(std::ostream& out) const {
    value->print(out);
}

void MatchSingleton::print(std::ostream& out) const {
    value.print(out);
}

void MatchSequence::print(std::ostream& out) const {
    auto result = join(",", patterns);
    out << "[" << result << "]";
}

void Arguments::print(std::ostream& out, int indent) const {
    for(auto& arg: args) {
        out << arg.arg;

        if (arg.annotation.has_value()) {
            out << ": ";
            arg.annotation.value()->print(out, indent);
        }
    }

    for(auto& kw: kwonlyargs) {
        out << kw.arg;

        if (kw.annotation.has_value()) {
            out << ": ";
            kw.annotation.value()->print(out, indent);
        }
    }
}

void Arg::print(std::ostream& out, int indent) const {
    out << arg;
    if (annotation.has_value()){
        out << ": ";
        annotation.value()->print(out, indent);
    }
}

} // lython
