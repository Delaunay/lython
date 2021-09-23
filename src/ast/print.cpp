#include "logging/logging.h"
#include "magic.h"
#include "sexpression.h"
#include "utilities/strings.h"

namespace lython {

#define GEN_INDENT() auto _C_idt = String(indent * 4, ' ');
#define INDENT()     _C_idt;

String Pattern::__str__() const {
    StringStream ss;
    print(ss);
    return ss.str();
}

String Node::__str__() const {
    StringStream ss;
    if (kind == NodeKind::Invalid) {
        error("Node is invalid");
    }
    print(ss, 0);
    return ss.str();
}

String Comprehension::__str__() const {
    StringStream ss;
    print(ss, 1);
    return ss.str();
}

void print_body(std::ostream &out, int indent, Array<StmtNode *> const &body,
                bool print_last = false) {
    int k = 0;
    for (auto &stmt: body) {
        k += 1;

        out << std::string(indent * 4, ' ');
        stmt->print(out, indent);

        //
        if (k + 1 < body.size() || print_last) {
            out << "\n";
        }
    }
}

template <typename T, typename... Args>
void sprint(std::ostream &out, T *node, Args... args) {
    if (node == nullptr) {
        out << "<nullptr>";
        return;
    }

    node->print(out, args...);
}

void print_op(std::ostream &out, BoolOperator op) {
    // clang-format off
    switch (op) {
    case BoolOperator::And:   out << " and "; return;
    case BoolOperator::Or:    out << " or " ; return;
    case BoolOperator::None:  out << " <Bool:None> " ; return;
    }
    // clang-format on
}

void print_op(std::ostream &out, BinaryOperator op, bool aug) {
    // clang-format off
    switch (op) {
    case BinaryOperator::Add:       out << " +"; break;
    case BinaryOperator::Sub:       out << " -"; break;
    case BinaryOperator::Mult:      out << " *"; break;
    case BinaryOperator::MatMult:   out << " @"; break;
    case BinaryOperator::Div:       out << " /"; break;
    case BinaryOperator::Mod:       out << " %"; break;
    case BinaryOperator::Pow:       out << " **";  break;
    case BinaryOperator::LShift:    out << " <<";  break;
    case BinaryOperator::RShift:    out << " >>";  break;
    case BinaryOperator::BitOr:     out << " |";   break;
    case BinaryOperator::BitXor:    out << " ^";   break;
    case BinaryOperator::BitAnd:    out << " &";   break;
    case BinaryOperator::FloorDiv:  out << " //";  break;
    case BinaryOperator::EltMult:   out << " .*";  break;
    case BinaryOperator::EltDiv:    out << " ./";  break;
    case BinaryOperator::None:      out << " <Binary:None>"; break;
    }
    // clang-format on

    if (!aug) {
        out << " ";
    }
}

void print_op(std::ostream &out, CmpOperator op) {
    // clang-format off
    switch (op) {
    case CmpOperator::None:     out << " <Cmp:None> "; return;
    case CmpOperator::Eq:       out << " == ";  return;
    case CmpOperator::NotEq:    out << " != ";  return;
    case CmpOperator::Lt:       out << " < ";   return;
    case CmpOperator::LtE:      out << " <= ";  return;
    case CmpOperator::Gt:       out << " > ";   return;
    case CmpOperator::GtE:      out << " >= ";  return;
    case CmpOperator::Is:       out << " is ";  return;
    case CmpOperator::IsNot:    out << " is not ";  return;
    case CmpOperator::In:       out << " in ";      return;
    case CmpOperator::NotIn:    out << " not in ";  return;
    }
    // clang-format on
}

void print_op(std::ostream &out, UnaryOperator op) {
    // clang-format off
    switch (op) {
    case UnaryOperator::None:   out << "<Unary:None>"; return;
    case UnaryOperator::Invert: out << "~"; return;
    case UnaryOperator::Not:    out << "!"; return;
    case UnaryOperator::UAdd:   out << "+"; return;
    case UnaryOperator::USub:   out << "-"; return;
    }
    // clang-format on
}

void Raise::print(std::ostream &out, int indent) const {
    out << "raise ";
    if (exc.has_value()) {
        exc.value()->print(out, indent);
    }

    if (cause.has_value()) {
        out << " from ";
        cause.value()->print(out, indent);
    }
}

void Assert::print(std::ostream &out, int indent) const {
    out << "assert ";
    test->print(out, indent);

    if (msg.has_value()) {
        out << ", ";
        msg.value()->print(out, indent);
    }
}

void With::print(std::ostream &out, int indent) const {
    out << "with ";

    int i = 0;
    for (auto &item: items) {
        item.context_expr->print(out, indent);

        if (item.optional_vars.has_value()) {
            out << " as ";
            item.optional_vars.value()->print(out);
        }

        if (i + 1 < items.size()) {
            out << ", ";
        }
        i += 1;
    }
    out << ":\n";

    print_body(out, indent + 1, body);
}

void ConstantValue::print(std::ostream &out) const {
    switch (kind) {
    case TInvalid:
        out << "<Constant:Invalid>";
    case TString:
        out << "\"" << value.string << "\"";

    case TFloat:
        out << value.single;

    case TDouble:
        out << value.decimal;

    case TInt:
        out << value.integer;
    }
}

void Import::print(std::ostream &out, int indent) const {
    out << "import ";

    int i = 0;
    for (auto &alias: names) {
        out << alias.name;

        if (alias.asname.has_value()) {
            out << " as ";
            out << alias.asname.value();
        }

        if (i + 1 < names.size()) {
            out << ", ";
        }
        i += 1;
    }
}

void ImportFrom::print(std::ostream &out, int indent) const {
    out << "from ";
    if (module.has_value()) {
        out << module.value();
    }
    out << " import ";

    int i = 0;
    for (auto &alias: names) {
        out << alias.name;

        if (alias.asname.has_value()) {
            out << " as ";
            out << alias.asname.value();
        }

        if (i + 1 < names.size()) {
            out << ", ";
        }
        i += 1;
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

void TupleExpr::print(std::ostream &out, int indent) const {
    if (indent == -1) {
        out << join<ExprNode *>(", ", elts);
    } else {
        out << "(" << join<ExprNode *>(", ", elts) << ")";
    }
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
    out << " for ";
    target->print(out, indent);
    out << " in ";
    iter->print(out, indent);

    for (auto expr: ifs) {
        out << " if ";
        expr->print(out, indent);
    }
}

void Keyword::print(std::ostream &out, int indent) const {
    out << arg;

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

void If::print(std::ostream &out, int indent) const {
    auto idt = String(indent * 4, ' ');

    out << "if ";
    test->print(out, indent);
    out << ":\n";
    print_body(out, indent + 1, body);

    if (orelse.size()) {
        out << "\n" << idt << "else:\n";
        print_body(out, indent + 1, orelse);
    }
}

void Match::print(std::ostream &out, int indent) const {
    out << "match ";
    subject->print(out, indent);
    out << ":\n";

    int i = 0;
    for (auto &case_: cases) {
        case_.print(out, indent + 1);

        if (i + 1 < cases.size()) {
            out << '\n';
        }
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
        auto &key = keywords[i];

        out << keywords[i].arg;
        out << "=";
        key.value->print(out, indent);

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

    if (vararg.has_value()) {
        if (args.size() > 0) {
            out << ", ";
        }

        out << "*" << vararg.value().arg;
    }

    if ((args.size() > 0 || vararg.has_value()) && kwonlyargs.size() > 0) {
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

    if (kwarg.has_value()) {
        if (kwonlyargs.size() > 0) {
            out << ", ";
        }
        out << "**" << kwarg.value().arg;
    }
}

void Arg::print(std::ostream &out, int indent) const {
    out << arg;
    if (annotation.has_value()) {
        out << ": ";
        annotation.value()->print(out, indent);
    }
}

void NamedExpr::print(std::ostream &out, int indent) const {
    target->print(out, indent);
    out << " := ";
    value->print(out, indent);
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

    lython::print_body(out, indent + 1, body, true);
}

void For::print(std::ostream &out, int indent) const {
    auto idt = String(indent * 4, ' ');

    out << idt << "for ";
    target->print(out, -1);
    out << " in ";
    sprint(out, iter, -1);
    out << ":\n";
    print_body(out, indent + 1, body);

    if (orelse.size() > 0) {
        out << '\n';
        out << idt << "else:\n";
        print_body(out, indent + 1, orelse);
    }
}

void ExceptHandler::print(std::ostream &out, int indent) const {
    auto idt = String(indent * 4, ' ');

    out << '\n' << idt << "except ";

    if (type.has_value()) {
        type.value()->print(out, indent);
    }

    if (name.has_value()) {
        out << " as ";
        out << name.value();
    }

    out << ":\n";
    lython::print_body(out, indent + 1, body);
}

void Try::print(std::ostream &out, int indent) const {
    auto idt = String(indent * 4, ' ');

    out << idt << "try:\n";
    print_body(out, indent + 1, body);

    for (auto &handler: handlers) {
        handler.print(out, indent);
    }

    if (orelse.size() > 0) {
        out << "\n" << idt << "else:\n";
        print_body(out, indent + 1, orelse);
    }

    if (finalbody.size() > 0) {
        out << "\n" << idt << "finally:\n";
        print_body(out, indent + 1, finalbody);
    }
}

void Compare::print(std::ostream &out, int indent) const {
    left->print(out, indent);

    for (int i = 0; i < ops.size(); i++) {
        print_op(out, ops[i]);
        comparators[i]->print(out, indent);
    }
}

int get_precedence(Node const *node) {
    if (node->kind == NodeKind::BinOp) {
        BinOp *op = (BinOp *)(node);
        // clang-format off
        switch (op->op) {
            case BinaryOperator::Add: return 1;
            case BinaryOperator::Sub: return 1;
            case BinaryOperator::Mult: return 2;
            case BinaryOperator::Div: return 2;
            case BinaryOperator::Pow: return 3;
            case BinaryOperator::BitXor: return 3;
        }
        // clang-format on
        return 10;
    }
    return 1000;
}

void BinOp::print(std::ostream &out, int indent) const {
    auto self    = get_precedence(this);
    auto lhspred = get_precedence(left) <= self;
    auto rhspred = get_precedence(right) <= self;

    if (lhspred) {
        out << '(';
    }
    left->print(out, indent);
    if (lhspred) {
        out << ')';
    }

    print_op(out, op);

    if (rhspred) {
        out << '(';
    }
    right->print(out, indent);
    if (rhspred) {
        out << ')';
    }
}

void BoolOp::print(std::ostream &out, int indent) const {
    values[0]->print(out, indent);
    print_op(out, op);
    values[1]->print(out, indent);
}

void UnaryOp::print(std::ostream &out, int indent) const {
    print_op(out, op);
    out << " ";
    operand->print(out, indent);
}

void While::print(std::ostream &out, int indent) const {
    auto idt = String(indent * 4, ' ');

    out << idt << "while ";
    test->print(out);
    out << ":\n";
    print_body(out, indent + 1, body);

    if (orelse.size() > 0) {
        out << '\n' << idt << "else:\n";
        print_body(out, indent + 1, orelse);
    }
}

void Return::print(std::ostream &out, int indent) const {
    out << "return ";

    if (value.has_value()) {
        value.value()->print(out, indent);
    }

    out << "\n";
}

void Delete::print(std::ostream &out, int indent) const {
    out << "del ";

    for (int i = 0; i < targets.size(); i++) {
        targets[i]->print(out, indent);

        if (i < targets.size() - 1)
            out << ", ";
    }
}

void AugAssign::print(std::ostream &out, int indent) const {
    sprint(out, target, -1);
    print_op(out, op, true);
    out << "= ";
    sprint(out, value, -1);
}

void Assign::print(std::ostream &out, int indent) const {
    targets[0]->print(out, -1);
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
        value->print(out, -1);
}

void Global::print(std::ostream &out, int indent) const { out << "global " << join(", ", names); }

void Nonlocal::print(std::ostream &out, int indent) const {
    out << "nonlocal " << join(", ", names);
}

} // namespace lython
