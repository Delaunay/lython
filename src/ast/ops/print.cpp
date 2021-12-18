#include "ast/magic.h"
#include "ast/nodes.h"
#include "ast/visitor.h"
#include "logging/logging.h"
#include "utilities/strings.h"

namespace lython {

struct PrintTrait {
    using StmtRet = bool;
    using ExprRet = bool;
    using ModRet  = bool;
    using PatRet  = bool;
};

int  get_precedence(Node const *node);
void print_op(std::ostream &out, UnaryOperator op);
void print_op(std::ostream &out, CmpOperator op);
void print_op(std::ostream &out, BinaryOperator op, bool aug);
void print_op(std::ostream &out, BoolOperator op);

#define ReturnType bool

struct Printer: BaseVisitor<Printer, true, PrintTrait, std::ostream &> {

    String l0;
    String latest = String(128, ' ');

    String const indent(int level, int scale = 4) {
        if (level <= 0)
            return l0;

        // one time allocation of an empty string
        latest.resize(scale * level, ' ');
        return latest;
    }

    ReturnType print_body(Array<StmtNode *> const &body, int level, std::ostream &out,
                          bool print_last = false) {

        int k = 0;
        for (auto &stmt: body) {
            k += 1;

            out << indent(level);
            exec(stmt, level, out);

            if (k + 1 < body.size() || print_last) {
                out << "\n";
            }
        }

        return true;
    }

    ReturnType excepthandler(ExceptHandler const &self, int level, std::ostream &out) {
        out << '\n' << indent(level) << "except ";

        if (self.type.has_value()) {
            exec(self.type.value(), level, out);
        }

        if (self.name.has_value()) {
            out << " as ";
            out << self.name.value();
        }

        out << ":\n";
        return print_body(self.body, level + 1, out);
    }

    ReturnType matchcase(MatchCase const &self, int level, std::ostream &out) {
        out << indent(level) << "case ";
        exec(self.pattern, level, out);

        if (self.guard.has_value()) {
            out << " if ";
            exec(self.guard.value(), level, out);
        }

        out << ":\n";
        return print_body(self.body, level + 1, out);
    }

    void arguments(Arguments const &self, int level, std::ostream &out);
    void withitem(WithItem const &self, int level, std::ostream &out);
    void alias(Alias const &self, int level, std::ostream &out);
    void keyword(Keyword const &self, int level, std::ostream &out);

    void arg(Arg const &self, int level, std::ostream &out) {
        out << self.arg;

        if (self.annotation.has_value()) {
            out << ": ";
            exec(self.annotation.value(), level, out);
        }
    }
#define FUNCTION_GEN(name, fun, rtype) rtype fun(const name *node, int depth, std::ostream &out);

#define X(name, _)
#define SECTION(name)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun, ReturnType)
#define STMT(name, fun)  FUNCTION_GEN(name, fun, ReturnType)
#define MOD(name, fun)   FUNCTION_GEN(name, fun, ReturnType)
#define MATCH(name, fun) FUNCTION_GEN(name, fun, ReturnType)

    NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH

#undef FUNCTION_GEN
};

void comprehension(Printer &p, Comprehension const &self, int level, std::ostream &out);

void print(Comprehension const &self, std::ostream &out) {
    Printer p;
    comprehension(p, self, 0, out);
}

void print(Node const *n, std::ostream &out) {
    auto p = Printer();
    p.exec<bool>(n, out);
}

ReturnType Printer::attribute(Attribute const *self, int indent, std::ostream &out) {
    exec(self->value, indent, out);
    out << ".";
    out << self->attr;
    return false;
}

ReturnType Printer::subscript(Subscript const *self, int indent, std::ostream &out) {
    exec(self->value, indent, out);
    out << "[";
    exec(self->slice, indent, out);
    out << "]";
    return false;
}

ReturnType Printer::starred(Starred const *self, int indent, std::ostream &out) {
    out << "*";
    exec(self->value, indent, out);
    return false;
}

ReturnType Printer::module(Module const *self, int level, std::ostream &out) {
    return print_body(self->body, level, out);
}

ReturnType Printer::raise(Raise const *self, int level, std::ostream &out) {
    out << "raise ";
    if (self->exc.has_value()) {
        exec(self->exc.value(), level, out);
    }

    if (self->cause.has_value()) {
        out << " from ";
        exec(self->cause.value(), level, out);
    }
    return false;
}

ReturnType Printer::assertstmt(Assert const *self, int level, std::ostream &out) {
    out << "assert ";
    exec(self->test, level, out);

    if (self->msg.has_value()) {
        out << ", ";
        exec(self->msg.value(), level, out);
    }
    return false;
}

ReturnType Printer::with(With const *self, int level, std::ostream &out) {
    out << "with ";

    int i = 0;
    for (auto &item: self->items) {
        exec(item.context_expr, level, out);

        if (item.optional_vars.has_value()) {
            out << " as ";
            exec(item.optional_vars.value(), level, out);
        }

        if (i + 1 < self->items.size()) {
            out << ", ";
        }
        i += 1;
    }
    out << ":\n";

    print_body(self->body, level + 1, out);
    return false;
}

ReturnType Printer::import(Import const *self, int level, std::ostream &out) {
    out << "import ";

    int i = 0;
    for (auto &alias: self->names) {
        out << alias.name;

        if (alias.asname.has_value()) {
            out << " as ";
            out << alias.asname.value();
        }

        if (i + 1 < self->names.size()) {
            out << ", ";
        }
        i += 1;
    }
    return false;
}

ReturnType Printer::importfrom(ImportFrom const *self, int level, std::ostream &out) {
    out << "from ";
    if (self->module.has_value()) {
        out << self->module.value();
    }
    out << " import ";

    int i = 0;
    for (auto &alias: self->names) {
        out << alias.name;

        if (alias.asname.has_value()) {
            out << " as ";
            out << alias.asname.value();
        }

        if (i + 1 < self->names.size()) {
            out << ", ";
        }
        i += 1;
    }
    return false;
}

ReturnType Printer::slice(Slice const *self, int level, std::ostream &out) {
    if (self->lower.has_value()) {
        exec(self->lower.value(), level, out);
    }

    out << ":";

    if (self->upper.has_value()) {
        exec(self->upper.value(), level, out);
    }

    if (self->step.has_value()) {
        out << ":";
        exec(self->step.value(), level, out);
    }
    return false;
}

ReturnType Printer::tupleexpr(TupleExpr const *self, int level, std::ostream &out) {
    if (level == -1) {
        out << join<ExprNode *>(", ", self->elts);
    } else {
        out << "(" << join<ExprNode *>(", ", self->elts) << ")";
    }
    return false;
}

ReturnType Printer::listexpr(ListExpr const *self, int level, std::ostream &out) {
    out << "[" << join<ExprNode *>(", ", self->elts) << "]";
    return false;
}

ReturnType Printer::setexpr(SetExpr const *self, int level, std::ostream &out) {
    out << "{" << join<ExprNode *>(", ", self->elts) << "}";
    return false;
}

ReturnType Printer::dictexpr(DictExpr const *self, int level, std::ostream &out) {
    Array<String> strs;
    strs.reserve(self->keys.size());

    for (int i = 0; i < self->keys.size(); i++) {
        // FIXME std::string -> String conversion
        strs.push_back(
            String(fmt::format("{}: {}", str(self->keys[i]), str(self->values[i])).c_str()));
    }

    out << "{" << join(", ", strs) << "}";
    return false;
}

ReturnType Printer::matchvalue(MatchValue const *self, int level, std::ostream &out) {
    exec(self->value, level, out);
    return false;
}

ReturnType Printer::matchsingleton(MatchSingleton const *self, int level, std::ostream &out) {
    self->value.print(out);
    return false;
}

ReturnType Printer::matchsequence(MatchSequence const *self, int level, std::ostream &out) {
    auto result = join(", ", self->patterns);
    out << "[" << result << "]";
    return false;
}

ReturnType Printer::matchmapping(MatchMapping const *self, int level, std::ostream &out) {
    Array<String> strs;
    strs.reserve(self->keys.size());

    for (int i = 0; i < self->keys.size(); i++) {
        // FIXME std::string -> String conversion
        strs.push_back(
            String(fmt::format("{}: {}", str(self->keys[i]), str(self->patterns[i])).c_str()));
    }

    String remains = "";
    if (self->rest.has_value()) {
        StringStream ss;
        ss << ", **" << self->rest.value();
        remains = ss.str();
    }

    out << "{" << join(", ", strs) << remains << "}";
    return false;
}

ReturnType Printer::matchclass(MatchClass const *self, int level, std::ostream &out) {
    exec(self->cls, level, out);
    out << "(" << join(", ", self->patterns);

    if (self->patterns.size() > 0 && self->kwd_attrs.size() > 0) {
        out << ", ";
    }

    Array<String> kwdpat;
    kwdpat.reserve(self->kwd_attrs.size());

    for (int i = 0; i < self->kwd_attrs.size(); i++) {
        // FIXME std::string -> String conversion
        kwdpat.push_back(
            String(fmt::format("{}={}", self->kwd_attrs[i], str(self->kwd_patterns[i])).c_str()));
    }

    out << join(", ", kwdpat);
    out << ")";
    return false;
}

ReturnType Printer::matchstar(MatchStar const *self, int level, std::ostream &out) {
    out << "*";

    if (self->name.has_value()) {
        out << self->name.value();
    }
    return false;
}

ReturnType Printer::matchas(MatchAs const *self, int level, std::ostream &out) {
    if (self->pattern.has_value()) {
        exec(self->pattern.value(), level, out);
    }

    if (self->name.has_value()) {
        out << " as " << self->name.value();
    }
    return false;
}

ReturnType Printer::matchor(MatchOr const *self, int level, std::ostream &out) {
    out << join(" | ", self->patterns);
    return false;
}

ReturnType Printer::ifstmt(If const *self, int level, std::ostream &out) {
    out << "if ";
    exec(self->test, level, out);
    out << ":\n";
    print_body(self->body, level + 1, out);

    for (int i = 0; i < self->tests.size(); i++) {
        auto &eliftest = self->tests[i];
        auto &elifbody = self->bodies[i];

        out << "\n" << indent(level) << "elif ";

        exec(eliftest, level, out);
        out << ":\n";
        print_body(elifbody, level + 1, out);
    }

    if (self->orelse.size()) {
        out << "\n" << indent(level) << "else:\n";
        print_body(self->orelse, level + 1, out);
    }
    return false;
}

ReturnType Printer::match(Match const *self, int level, std::ostream &out) {
    out << "match ";
    exec(self->subject, level, out);
    out << ":\n";

    int i = 0;
    for (auto &case_: self->cases) {
        matchcase(case_, level + 1, out);

        if (i + 1 < self->cases.size()) {
            out << '\n';
        }
    }
    return false;
}

ReturnType Printer::lambda(Lambda const *self, int level, std::ostream &out) {
    out << "lambda ";
    arguments(self->args, 0, out);
    out << ": ";
    exec(self->body, level, out);
    return false;
}

ReturnType Printer::ifexp(IfExp const *self, int level, std::ostream &out) {
    out << "if ";
    exec(self->test, level, out);
    out << ": ";
    exec(self->body, level, out);
    out << " else ";
    exec(self->orelse, level, out);
    return false;
}

ReturnType Printer::listcomp(ListComp const *self, int level, std::ostream &out) {
    out << "[";
    exec(self->elt, level, out);

    out << join(" ", self->generators);

    out << "]";
    return false;
}

ReturnType Printer::setcomp(SetComp const *self, int level, std::ostream &out) {
    out << "{";
    exec(self->elt, level, out);

    out << join(" ", self->generators);

    out << "}";
    return false;
}

ReturnType Printer::generateexpr(GeneratorExp const *self, int level, std::ostream &out) {
    out << "(";
    exec(self->elt, level, out);

    out << join(" ", self->generators);

    out << ")";
    return false;
}

ReturnType Printer::dictcomp(DictComp const *self, int level, std::ostream &out) {
    out << "{";
    exec(self->key, level, out);
    out << ": ";
    exec(self->value, level, out);

    out << join(" ", self->generators);
    out << "}";
    return false;
}

ReturnType Printer::await(Await const *self, int level, std::ostream &out) {
    out << "await ";
    exec(self->value, level, out);
    return false;
}

ReturnType Printer::yield(Yield const *self, int level, std::ostream &out) {
    out << "yield ";
    if (self->value.has_value()) {
        exec(self->value.value(), level, out);
    }
    return false;
}

ReturnType Printer::yieldfrom(YieldFrom const *self, int level, std::ostream &out) {
    out << "yield from ";
    exec(self->value, level, out);
    return false;
}

ReturnType Printer::call(Call const *self, int level, std::ostream &out) {
    exec(self->func, level, out);
    out << "(";

    for (int i = 0; i < self->args.size(); i++) {
        exec(self->args[i], level, out);

        if (i < self->args.size() - 1 || self->keywords.size() > 0)
            out << ", ";
    }

    for (int i = 0; i < self->keywords.size(); i++) {
        auto &key = self->keywords[i];

        out << self->keywords[i].arg;
        out << "=";

        exec(key.value, level, out);

        if (i < self->keywords.size() - 1)
            out << ", ";
    }

    out << ")";
    return false;
}

ReturnType Printer::constant(Constant const *self, int level, std::ostream &out) {
    self->value.print(out);
    return false;
}

ReturnType Printer::namedexpr(NamedExpr const *self, int level, std::ostream &out) {
    exec(self->target, level, out);
    out << " := ";
    exec(self->value, level, out);
    return false;
}

ReturnType Printer::classdef(ClassDef const *self, int level, std::ostream &out) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out << indent(level);
        }

        out << "@";
        exec(decorator, level, out);
        out << "\n";
        k += 1;
    }

    if (self->decorator_list.size() > 0) {
        out << indent(level);
    }

    out << "class " << self->name;
    if (self->bases.size() + self->keywords.size() > 0) {
        out << '(';
    }

    out << join<ExprNode *>(", ", self->bases);

    if (self->bases.size() > 0 && self->keywords.size() > 0) {
        out << ", ";
    }

    Array<String> kwd;
    kwd.reserve(self->keywords.size());

    for (auto kw: self->keywords) {
        // FIXME std::string -> String conversion
        kwd.push_back(String(fmt::format("{}={}", str(kw.arg), str(kw.value)).c_str()));
    }

    out << join(", ", kwd);

    if (self->bases.size() + self->keywords.size() > 0) {
        out << ')';
    }

    out << ":\n";

    if (self->docstring.has_value()) {
        out << indent(level + 1) << "\"\"\"" << self->docstring.value() << "\"\"\"\n";
    }

    print_body(self->body, level + 1, out);
    return false;
}

ReturnType Printer::functiondef(FunctionDef const *self, int level, std::ostream &out) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out << indent(level);
        }

        out << "@";
        exec(decorator, level, out);
        out << "\n";
        k += 1;
    }

    if (self->decorator_list.size() > 0) {
        out << indent(level);
    }

    out << "def " << self->name << "(";
    arguments(self->args, level, out);
    out << ")";

    if (self->returns.has_value()) {
        out << " -> ";
        exec(self->returns.value(), level, out);
    }

    out << ":\n";

    if (self->docstring.has_value()) {
        out << indent(level + 1) << "\"\"\"" << self->docstring.value() << "\"\"\"\n";
    }

    print_body(self->body, level + 1, out, true);

    out << "\n";
    return false;
}

ReturnType Printer::inlinestmt(Inline const *self, int level, std::ostream &out) {
    out << indent(level);

    int k = 0;
    for (auto &stmt: self->body) {
        exec(stmt, level, out);

        if (k + 1 < self->body.size()) {
            out << "; ";
        }

        k += 1;
    }

    return false;
}

ReturnType Printer::forstmt(For const *self, int level, std::ostream &out) {
    out << "for ";
    exec(self->target, -1, out);
    out << " in ";
    exec(self->iter, -1, out);
    out << ":\n";
    print_body(self->body, level + 1, out);

    if (self->orelse.size() > 0) {
        out << '\n';
        out << indent(level) << "else:\n";
        print_body(self->orelse, level + 1, out);
    }

    return false;
}

ReturnType Printer::trystmt(Try const *self, int level, std::ostream &out) {
    out << "try:\n";
    print_body(self->body, level + 1, out);

    for (auto &handler: self->handlers) {
        excepthandler(handler, level, out);
    }

    if (self->orelse.size() > 0) {
        out << "\n" << indent(level) << "else:\n";
        print_body(self->orelse, level + 1, out);
    }

    if (self->finalbody.size() > 0) {
        out << "\n" << indent(level) << "finally:\n";
        print_body(self->finalbody, level + 1, out);
    }

    return false;
}

ReturnType Printer::compare(Compare const *self, int level, std::ostream &out) {
    exec(self->left, level, out);

    for (int i = 0; i < self->ops.size(); i++) {
        print_op(out, self->ops[i]);
        exec(self->comparators[i], level, out);
    }

    return false;
}

ReturnType Printer::binop(BinOp const *self, int level, std::ostream &out) {
    auto sself   = get_precedence(self);
    auto lhspred = get_precedence(self->left) < sself;
    auto rhspred = get_precedence(self->right) < sself;

    if (lhspred) {
        out << '(';
    }
    exec(self->left, level, out);
    if (lhspred) {
        out << ')';
    }

    print_op(out, self->op, false);

    if (rhspred) {
        out << '(';
    }
    exec(self->right, level, out);
    if (rhspred) {
        out << ')';
    }

    return false;
}

ReturnType Printer::boolop(BoolOp const *self, int level, std::ostream &out) {
    exec(self->values[0], level, out);
    print_op(out, self->op);
    exec(self->values[1], level, out);
    return false;
}

ReturnType Printer::unaryop(UnaryOp const *self, int level, std::ostream &out) {
    print_op(out, self->op);
    out << " ";
    exec(self->operand, level, out);

    return false;
}

ReturnType Printer::whilestmt(While const *self, int level, std::ostream &out) {
    out << "while ";
    exec(self->test, level, out);
    out << ":\n";
    print_body(self->body, level + 1, out);

    if (self->orelse.size() > 0) {
        out << '\n' << indent(level) << "else:\n";
        print_body(self->orelse, level + 1, out);
    }

    return false;
}

ReturnType Printer::returnstmt(Return const *self, int level, std::ostream &out) {
    out << "return ";

    if (self->value.has_value()) {
        exec(self->value.value(), level, out);
    }

    return false;
}

ReturnType Printer::deletestmt(Delete const *self, int level, std::ostream &out) {
    out << "del ";

    for (int i = 0; i < self->targets.size(); i++) {
        exec(self->targets[i], level, out);

        if (i < self->targets.size() - 1)
            out << ", ";
    }

    return false;
}

ReturnType Printer::augassign(AugAssign const *self, int level, std::ostream &out) {
    exec(self->target, -1, out);
    print_op(out, self->op, true);
    out << "= ";
    exec(self->value, -1, out);
    return false;
}

ReturnType Printer::assign(Assign const *self, int level, std::ostream &out) {
    exec(self->targets[0], -1, out);
    out << " = ";
    exec(self->value, -1, out);
    return false;
}

ReturnType Printer::annassign(AnnAssign const *self, int level, std::ostream &out) {
    exec(self->target, level, out);
    out << ": ";

    exec(self->annotation, level, out);
    if (self->value.has_value()) {
        out << " = ";
        exec(self->value.value(), level, out);
    }

    return false;
}

ReturnType Printer::pass(Pass const *self, int level, std::ostream &out) {
    out << "pass";
    return false;
}

ReturnType Printer::breakstmt(Break const *self, int level, std::ostream &out) {
    out << "break";
    return false;
}

ReturnType Printer::continuestmt(Continue const *self, int level, std::ostream &out) {
    out << "continue";
    return false;
}

ReturnType Printer::exprstmt(Expr const *self, int level, std::ostream &out) {
    if (self->value != nullptr)
        exec(self->value, -1, out);

    return false;
}

ReturnType Printer::global(Global const *self, int level, std::ostream &out) {
    out << "global " << join(", ", self->names);
    return false;
}

ReturnType Printer::nonlocal(Nonlocal const *self, int level, std::ostream &out) {
    out << "nonlocal " << join(", ", self->names);
    return false;
}

ReturnType Printer::arrow(Arrow const *self, int level, std::ostream &out) {
    out << '(' << join<ExprNode *>(", ", self->args) << ") -> ";
    out << str(self->returns);
    return false;
}

ReturnType Printer::dicttype(DictType const *self, int indent, std::ostream &out) {
    out << "Dict[";
    out << str(self->key);
    out << ", ";
    out << str(self->value) << "]";
    return false;
}

ReturnType Printer::settype(SetType const *self, int indent, std::ostream &out) {
    out << "Set[";
    out << str(self->value) << "]";
    return false;
}

ReturnType Printer::name(Name const *self, int indent, std::ostream &out) {
    out << self->id;
    return false;
}

ReturnType Printer::arraytype(ArrayType const *self, int indent, std::ostream &out) {
    out << "Array[";
    out << str(self->value) << "]";
    return false;
}

ReturnType Printer::tupletype(TupleType const *self, int indent, std::ostream &out) {
    out << "Tuple[";
    out << join<ExprNode *>(", ", self->types) << "]";
    return false;
}

ReturnType Printer::builtintype(BuiltinType const *self, int indent, std::ostream &out) {
    out << self->name;
    return false;
}

ReturnType Printer::joinedstr(JoinedStr const *self, int indent, std::ostream &out) {
    out << "JoinedStr";
    return false;
}

ReturnType Printer::formattedvalue(FormattedValue const *self, int indent, std::ostream &out) {
    out << "FormattedValue";
    return false;
}

ReturnType Printer::classtype(ClassType const *self, int indent, std::ostream &out) {
    out << self->def->name;
    return false;
}

// Helper
// ==================================================

void ConstantValue::print(std::ostream &out) const {
    switch (kind) {
    case TInvalid:
        out << "<Constant:Invalid>";
        break;

    case TInt:
        out << value.integer;
        break;

    case TFloat:
        out << value.singlef;
        break;

    case TDouble:
        // always print a float even without decimal point
        out << fmt::format("{:#}", value.doublef);
        break;

    case TBool:
        if (value.boolean) {
            out << "True";
        } else {
            out << "False";
        }
        break;

    case TNone:
        out << "None";
        break;

    case TString:
        out << "\"" << value.string << "\"";
        break;
    }
}

String Node::__str__() const {
    StringStream ss;
    if (kind == NodeKind::Invalid) {
        error("Node is invalid");
    }
    Printer p;
    p.exec<bool>(this, ss);
    return ss.str();
}

// void Node::print(std::ostream &out, int indent) const {
//     out << "<not-implemented:";
//     out << str(kind);
//     out << ">";
// }

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

void comprehension(Printer &p, Comprehension const &self, int level, std::ostream &out) {
    out << " for ";
    p.exec(self.target, level, out);
    out << " in ";
    p.exec(self.iter, level, out);

    for (auto expr: self.ifs) {
        out << " if ";
        p.exec(expr, level, out);
    }
}

void Printer::keyword(Keyword const &self, int level, std::ostream &out) {
    out << self.arg;
    if (self.value != nullptr) {
        out << " = ";
        exec(self.value, level, out);
    }
}

void Printer::alias(Alias const &self, int level, std::ostream &out) {
    out << self.name;
    if (self.asname.has_value()) {
        out << " as " << self.asname.value();
    }
}

ReturnType Printer::functiontype(FunctionType const *self, int level, std::ostream &out) {
    return true;
}

ReturnType Printer::expression(Expression const *self, int level, std::ostream &out) {
    return true;
}

ReturnType Printer::interactive(Interactive const *self, int level, std::ostream &out) {
    return true;
}

void Printer::withitem(WithItem const &self, int level, std::ostream &out) {
    exec(self.context_expr, level, out);
    if (self.optional_vars.has_value()) {
        out << " as ";
        exec(self.optional_vars.value(), level, out);
    }
}

void Printer::arguments(Arguments const &self, int level, std::ostream &out) {
    int i = 0;

    for (auto &arg: self.args) {
        out << arg.arg;

        if (arg.annotation.has_value()) {
            out << ": ";
            exec(arg.annotation.value(), level, out);
        }

        auto default_offset = self.args.size() - 1 - i;
        if (self.defaults.size() > 0 && default_offset < self.defaults.size()) {
            if (arg.annotation.has_value()) {
                out << " = ";
            } else {
                out << "=";
            }
            exec(self.defaults[default_offset], -1, out);
        }

        if (i + 1 < self.args.size()) {
            out << ", ";
        }
        i += 1;
    }

    if (self.vararg.has_value()) {
        if (self.args.size() > 0) {
            out << ", ";
        }

        out << "*" << self.vararg.value().arg;
    }

    if ((self.args.size() > 0 || self.vararg.has_value()) && self.kwonlyargs.size() > 0) {
        out << ", ";
    }

    i = 0;
    for (auto &kw: self.kwonlyargs) {
        out << kw.arg;

        if (kw.annotation.has_value()) {
            out << ": ";
            exec(kw.annotation.value(), level, out);
        }

        auto default_offset = self.kwonlyargs.size() - 1 - i;
        if (self.kw_defaults.size() > 0 && default_offset < self.kw_defaults.size()) {
            if (kw.annotation.has_value()) {
                out << " = ";
            } else {
                out << "=";
            }
            exec(self.kw_defaults[default_offset], -1, out);
        }

        if (i + 1 < self.kwonlyargs.size()) {
            out << ", ";
        }
        i += 1;
    }

    if (self.kwarg.has_value()) {
        if (self.kwonlyargs.size() > 0) {
            out << ", ";
        }
        out << "**" << self.kwarg.value().arg;
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

} // namespace lython
