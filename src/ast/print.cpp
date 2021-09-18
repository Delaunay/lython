#include "magic.h"
#include "sexpression.h"
#include "utilities/strings.h"

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

String Comprehension::__str__() const {
    StringStream ss;
    print(ss, 0);
    return ss.str();
}

void ConstantValue::print(std::ostream &out) const {
    switch (kind) {
    case TString:
        out << "\"" << value.string << "\"";

    case TFloat:
        out << value.decimal;

    case TInt:
        out << value.integer;
    }
}

void Slice::print(std::ostream &out, int indent) const {
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

void print_body(std::ostream &out, int indent, Array<StmtNode *> const &body) {
    for (auto &stmt: body) {
        out << std::string(indent * 4, ' ');
        stmt->print(out, indent);
        out << "\n";
    }
}

void ExceptHandler::print(std::ostream &out, int indent) const {
    out << "except ";

    if (type.has_value()) {
        type.value()->print(out, indent);
    }

    if (name.has_value()) {
        out << name.value();
    }

    out << ":\n";
    lython::print_body(out, indent + 1, body);
}

void TupleExpr::print(std::ostream &out, int indent) const {
    out << "(" << join<ExprNode *>(", ", elts) << ")";
}

void ListExpr::print(std::ostream &out, int indent) const {
    out << "[" << join<ExprNode *>(", ", elts) << "]";
}

void SetExpr::print(std::ostream &out, int indent) const {
    out << "{" << join<ExprNode *>(", ", elts) << "}";
}

void DictExpr::print(std::ostream &out, int indent) const {
    Array<String> strs;
    strs.reserve(keys.size());

    for (int i = 0; i < keys.size(); i++) {
        // FIXME std::string -> String conversion
        strs.push_back(String(fmt::format("{}: {}", str(keys[i]), str(values[i])).c_str()));
    }

    out << "{" << join(", ", strs) << "}";
}

void Comprehension::print(std::ostream &out, int indent) const {
    out << "for ";
    target->print(out, indent);
    out << " in ";
    iter->print(out, indent);

    for (auto expr: ifs) {
        out << "if ";
        expr->print(out, indent);
    }
}

void Keyword::print(std::ostream &out, int indent) const {
    if (arg.has_value()) {
        out << arg.value();
    }

    if (value != nullptr) {
        out << " = ";
        value->print(out, indent);
    }
}

void Alias::print(std::ostream &out, int indent) const {
    out << name;
    if (asname.has_value()) {
        out << " as " << asname.value();
    }
}

void WithItem::print(std::ostream &out, int indent) const {
    context_expr->print(out, indent);
    if (optional_vars.has_value()) {
        out << " as ";
        optional_vars.value()->print(out, indent);
    }
}

void MatchValue::print(std::ostream &out) const { value->print(out); }

void MatchSingleton::print(std::ostream &out) const { value.print(out); }

void MatchSequence::print(std::ostream &out) const {
    auto result = join(",", patterns);
    out << "[" << result << "]";
}

void MatchMapping::print(std::ostream &out) const {
    Array<String> strs;
    strs.reserve(keys.size());

    for (int i = 0; i < keys.size(); i++) {
        // FIXME std::string -> String conversion
        strs.push_back(String(fmt::format("{}: {}", str(keys[i]), str(patterns[i])).c_str()));
    }

    out << "{" << join(", ", strs) << "}";
}

void MatchClass::print(std::ostream &out) const {
    cls->print(out);
    out << "(" << join(", ", patterns);

    if (patterns.size() > 0 && kwd_attrs.size() > 0) {
        out << ",";
    }

    Array<String> kwdpat;
    kwdpat.reserve(kwd_attrs.size());

    for (int i = 0; i < kwd_attrs.size(); i++) {
        // FIXME std::string -> String conversion
        kwdpat.push_back(String(fmt::format("{}={}", kwd_attrs[i], str(kwd_patterns[i])).c_str()));
    }

    out << join(", ", kwdpat);
    out << ")";
}

void MatchStar::print(std::ostream &out) const {
    out << "*";

    if (name.has_value()) {
        out << name.value();
    }
}

void MatchAs::print(std::ostream &out) const {
    if (pattern.has_value()) {
        pattern.value()->print(out);
    }

    if (name.has_value()) {
        out << " as " << name.value();
    }
}

void MatchOr::print(std::ostream &out) const { out << join(" | ", patterns); }

void Module::print(std::ostream &out, int indent) const { print_body(out, indent, body); }

void MatchCase::print(std::ostream &out, int indent) const {
    out << String(4 * indent, ' ') << "case ";
    pattern->print(out);

    if (guard.has_value()) {
        out << " if ";
        guard.value()->print(out);
    }

    out << ":\n";
    print_body(out, indent + 1, body);
}

void Match::print(std::ostream &out, int indent) const {
    out << "match ";
    subject->print(out, indent);
    out << ":\n";

    for (auto &case_: cases) {
        case_.print(out, indent + 1);
    }
}

void Lambda::print(std::ostream &out, int indent) const {
    out << "lambda ";
    args.print(out, 0);
    out << ": ";
    body->print(out, indent);
}

void IfExp::print(std::ostream &out, int indent) const {
    out << "if ";
    test->print(out);
    out << ": ";
    body->print(out);
    out << " else ";
    orelse->print(out, indent);
}

void ListComp::print(std::ostream &out, int indent) const {
    out << "[";
    elt->print(out);

    out << join(" ", generators);

    out << "]";
}

void SetComp::print(std::ostream &out, int indent) const {
    out << "{";
    elt->print(out);

    out << join(" ", generators);

    out << "}";
}

void GeneratorExp::print(std::ostream &out, int indent) const {
    out << "(";
    elt->print(out);

    out << join(" ", generators);

    out << ")";
}

void DictComp::print(std::ostream &out, int indent) const {
    out << "{";
    key->print(out);
    out << ": ";
    value->print(out);

    out << join(" ", generators);
    out << "}";
}

void Await::print(std::ostream &out, int indent) const {
    out << "await ";
    value->print(out);
}

void Yield::print(std::ostream &out, int indent) const {
    out << "yield ";
    if (value.has_value()) {
        value.value()->print(out);
    }
}

void YieldFrom::print(std::ostream &out, int indent) const {
    out << "yield from ";
    value->print(out);
}

void Call::print(std::ostream &out, int indent) const {
    func->print(out, indent);
    out << "(";

    for (int i = 0; i < args.size(); i++) {
        args[i]->print(out, indent);

        if (i < args.size() - 1 || keywords.size() > 0)
            out << ", ";
    }

    for (int i = 0; i < keywords.size(); i++) {
        out << keywords[i].arg.value();
        out << " = ";
        keywords[i].value->print(out, indent);

        if (i < keywords.size() - 1)
            out << ", ";
    }

    out << ")";
}

void Constant::print(std::ostream &out, int indent) const { value.print(out); }

void Arguments::print(std::ostream &out, int indent) const {
    int i = 0;
    for (auto &arg: args) {
        out << arg.arg;

        if (arg.annotation.has_value()) {
            out << ": ";
            arg.annotation.value()->print(out, indent);
        }

        if (i + 1 < args.size()) {
            out << ", ";
        }
        i += 1;
    }

    if (args.size() > 0 && kwonlyargs.size() > 0) {
        out << ", ";
    }

    i = 0;
    for (auto &kw: kwonlyargs) {
        out << kw.arg;

        if (kw.annotation.has_value()) {
            out << ": ";
            kw.annotation.value()->print(out, indent);
        }

        if (i + 1 < kwonlyargs.size()) {
            out << ", ";
        }
        i += 1;
    }
}

void Arg::print(std::ostream &out, int indent) const {
    out << arg;
    if (annotation.has_value()) {
        out << ": ";
        annotation.value()->print(out, indent);
    }
}

void ClassDef::print(std::ostream &out, int indent) const {
    out << "class " << name;
    if (bases.size() + keywords.size() > 0) {
        out << '(';
    }

    out << join<ExprNode *>(", ", bases);

    if (bases.size() > 0 && keywords.size() > 0) {
        out << ", ";
    }

    Array<String> kwd;
    kwd.reserve(keywords.size());

    for (auto kw: keywords) {
        // FIXME std::string -> String conversion
        kwd.push_back(String(fmt::format("{}={}", str(kw.arg), str(kw.value)).c_str()));
    }

    out << join(", ", kwd);

    if (bases.size() + keywords.size() > 0) {
        out << ')';
    }

    out << ":\n";

    print_body(out, indent + 1, body);
}

void FunctionDef::print(std::ostream &out, int indent) const {
    out << "def " << name << "(";

    args.print(out, indent);
    out << ")";

    if (returns.has_value()) {
        out << " -> ";
        returns.value()->print(out, indent);
    }

    out << ":\n";

    if (docstring.size() > 0) {
        out << String((indent + 1) * 4, ' ') << "\"\"\"" << docstring << "\"\"\"\n";
    }

    lython::print_body(out, indent + 1, body);
}

void For::print(std::ostream &out, int indent) const {
    out << "for ";
    target->print(out);
    out << " in ";
    iter->print(out);
    out << ":\n";
    print_body(out, indent + 1, body);

    if (orelse.size() > 0) {
        out << String(indent * 4, ' ') << "else:\n";
        print_body(out, indent + 1, body);
    }
}

void While::print(std::ostream &out, int indent) const {
    out << "while ";
    test->print(out);
    out << ":\n";
    print_body(out, indent + 1, body);

    if (orelse.size() > 0) {
        out << String(indent * 4, ' ') << "else:\n";
        print_body(out, indent + 1, body);
    }
}

void Return::print(std::ostream &out, int indent) const {
    out << "return ";

    if (value.has_value()) {
        value.value()->print(out, indent);
    }
}

void Delete::print(std::ostream &out, int indent) const {
    out << "del ";

    for (int i = 0; i < targets.size(); i++) {
        targets[i]->print(out, indent);

        if (i < targets.size() - 1)
            out << ", ";
    }
}

void Assign::print(std::ostream &out, int indent) const {
    targets[0]->print(out, indent);
    out << " = ";
    value->print(out, indent);
}

void AnnAssign::print(std::ostream &out, int indent) const {
    target->print(out, indent);
    out << ": ";
    annotation->print(out, indent);
    if (value.has_value()) {
        out << " = ";
        value.value()->print(out, indent);
    }
}

void Pass::print(std::ostream &out, int indent) const { out << "pass"; }

void Break::print(std::ostream &out, int indent) const { out << "break"; }

void Continue::print(std::ostream &out, int indent) const { out << "continue"; }

void Expr::print(std::ostream &out, int indent) const {
    if (value != nullptr)
        value->print(out, indent);
}

void Global::print(std::ostream &out, int indent) const { out << "global " << join(", ", names); }

void Nonlocal::print(std::ostream &out, int indent) const {
    out << "nonlocal " << join(", ", names);
}

} // namespace lython
