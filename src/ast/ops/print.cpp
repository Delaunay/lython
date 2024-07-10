#include "utilities/printing.h"

#include "ast/nodes.h"
#include "ast/visitor.h"
#include "dependencies/fmt.h"
#include "lexer/unlex.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/allocator.h"
#include "utilities/strings.h"

#include "dependencies/formatter.h"

namespace lython {

struct PrintTrait {
    using Trace   = std::false_type;
    using StmtRet = bool;
    using ExprRet = bool;
    using ModRet  = bool;
    using PatRet  = bool;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

int  get_precedence(Node const* node);

std::ostream& operator<<(std::ostream& out, Comprehension const& self);

void print_op(std::ostream& out, UnaryOperator op);
void print_op(std::ostream& out, CmpOperator op);
void print_op(std::ostream& out, BinaryOperator op, bool aug);
void print_op(std::ostream& out, BoolOperator op);

#define ReturnType bool

struct PrinterConfig {
    //
    int max_line;
};

struct PrinterContext {
    //
};

// Change this to return strings so we can change the format of partial results

struct Printer: BaseVisitor<Printer, true, PrintTrait, std::ostream&, int> {
    using Super = BaseVisitor<Printer, true, PrintTrait, std::ostream&, int>;

    PrinterContext context;
    String         l0;
    String         latest = String(128, ' ');

    String const indent(int level, int scale = 4) {
        if (level <= 0)
            return l0;

        // one time allocation of an empty string
        latest.resize(std::size_t(scale * level), ' ');
        return latest;
    }

    void maybe_inline_comment(
        Comment* com, int depth, std::ostream& out, int level, CodeLocation const& loc) {
        if (com != nullptr) {
            // lython::log(lython::LogLevel::Info, loc, "printing inline comment {}", com->comment);
            out << "   ";
            exec(com, depth, out, level);
        }
    }

    StringStream fmt(ExprNode const* node, int depth, int level) {
        StringStream ss;
        exec(node, depth, ss, level);
        return ss;
    }

    ReturnType print_body(Array<StmtNode*> const& body,
                          int                     depth,
                          std::ostream&           out,
                          int                     level,
                          bool                    print_last = false) {

        int k = 0;
        for (auto const& stmt: body) {
            k += 1;

            if (stmt == nullptr) {
                continue;
            }

            out << indent(level);
            bool printed_new_line = exec(stmt, depth, out, level);

            if (stmt->is_one_line()) {
                maybe_inline_comment(stmt->comment, depth, out, level, LOC);
            }

            // out << "\n";
            if (!printed_new_line) {
                if (k < body.size() || print_last) {
                    out << "\n";
                }
            }
        }

        return true;
    }

    ReturnType excepthandler(ExceptHandler const& self, int depth, std::ostream& out, int level) {
        out << '\n' << indent(level) << "except ";

        if (self.type.has_value()) {
            exec(self.type.value(), depth, out, level);
        }

        if (self.name.has_value()) {
            out << " as ";
            out << self.name.value();
        }

        out << ":";
        maybe_inline_comment(self.comment, depth, out, level, LOC);
        out << "\n";
        return print_body(self.body, depth, out, level + 1);
    }

    ReturnType matchcase(MatchCase const& self, int depth, std::ostream& out, int level) {
        out << indent(level) << "case ";
        exec(self.pattern, depth, out, level);

        if (self.guard.has_value()) {
            out << " if ";
            exec(self.guard.value(), depth, out, level);
        }

        out << ":";
        maybe_inline_comment(self.comment, depth, out, level, LOC);
        out << "\n";
        return print_body(self.body, depth, out, level + 1);
    }

    void arguments(Arguments const& self, int depth, std::ostream& out, int level);
    void withitem(WithItem const& self, int depth, std::ostream& out, int level);
    void alias(Alias const& self, int depth, std::ostream& out, int level);
    void keyword(Keyword const& self, int depth, std::ostream& out, int level);

    void arg(Arg const& self, int depth, std::ostream& out, int level) {
        out << self.arg;

        if (self.annotation.has_value()) {
            out << ": ";
            exec(self.annotation.value(), depth, out, level);
        }
    }
#define FUNCTION_GEN(name, fun) \
    ReturnType fun(const name* node, int depth, std::ostream& out, int level);

    KW_FOREACH_ALL(FUNCTION_GEN)

#undef FUNCTION_GEN
};

void comprehension(Printer& p, Comprehension const& self, int depth, std::ostream& out, int level);

std::ostream& operator<<(std::ostream& out, Comprehension const& self) {
    Printer p;
    comprehension(p, self, 0, out, 0);
    return out;
}

ReturnType Printer::attribute(Attribute const* self, int depth, std::ostream& out, int level) {
    exec(self->value, depth, out, level);
    out << ".";
    out << self->attr;
    return false;
}

ReturnType Printer::subscript(Subscript const* self, int depth, std::ostream& out, int level) {
    exec(self->value, depth, out, level);
    out << "[";
    exec(self->slice, depth, out, level);
    out << "]";
    return false;
}

ReturnType Printer::starred(Starred const* self, int depth, std::ostream& out, int level) {
    out << "*";
    exec(self->value, depth, out, level);
    return false;
}

ReturnType Printer::module(Module const* self, int depth, std::ostream& out, int level) {
    return print_body(self->body, depth, out, level);
}

ReturnType Printer::raise(Raise const* self, int depth, std::ostream& out, int level) {
    out << "raise ";
    if (self->exc.has_value()) {
        exec(self->exc.value(), depth, out, level);
    }

    if (self->cause.has_value()) {
        out << " from ";
        exec(self->cause.value(), depth, out, level);
    }
    return false;
}

ReturnType Printer::assertstmt(Assert const* self, int depth, std::ostream& out, int level) {
    out << "assert ";
    exec(self->test, depth, out, level);

    if (self->msg.has_value()) {
        out << ", ";
        exec(self->msg.value(), depth, out, level);
    }
    return false;
}

ReturnType Printer::with(With const* self, int depth, std::ostream& out, int level) {
    out << "with ";

    int i = 0;
    for (auto const& item: self->items) {
        exec(item.context_expr, depth, out, level);

        if (item.optional_vars.has_value()) {
            out << " as ";
            exec(item.optional_vars.value(), depth, out, level);
        }

        if (i + 1 < self->items.size()) {
            out << ", ";
        }
        i += 1;
    }
    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << "\n";

    print_body(self->body, depth, out, level + 1);
    return false;
}

ReturnType Printer::import(Import const* self, int depth, std::ostream& out, int level) {
    out << "import ";

    int i = 0;
    for (auto const& alias: self->names) {
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

ReturnType Printer::importfrom(ImportFrom const* self, int depth, std::ostream& out, int level) {
    out << "from ";
    if (self->module.has_value()) {
        out << self->module.value();
    }
    out << " import ";

    int i = 0;
    for (auto const& alias: self->names) {
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

ReturnType Printer::slice(Slice const* self, int depth, std::ostream& out, int level) {
    if (self->lower.has_value()) {
        exec(self->lower.value(), depth, out, level);
    }

    out << ":";

    if (self->upper.has_value()) {
        exec(self->upper.value(), depth, out, level);
    }

    if (self->step.has_value()) {
        out << ":";
        exec(self->step.value(), depth, out, level);
    }
    return false;
}

ReturnType Printer::tupleexpr(TupleExpr const* self, int depth, std::ostream& out, int level) {
    if (level == -1) {
        out << join<ExprNode*>(", ", self->elts);
    } else {
        out << "(" << join<ExprNode*>(", ", self->elts) << ")";
    }
    return false;
}

ReturnType Printer::listexpr(ListExpr const* self, int depth, std::ostream& out, int level) {
    out << "[" << join<ExprNode*>(", ", self->elts) << "]";
    return false;
}

ReturnType Printer::setexpr(SetExpr const* self, int depth, std::ostream& out, int level) {
    out << "{" << join<ExprNode*>(", ", self->elts) << "}";
    return false;
}

ReturnType Printer::dictexpr(DictExpr const* self, int depth, std::ostream& out, int level) {
    Array<String> strs;
    strs.reserve(self->keys.size());

    for (int i = 0; i < self->keys.size(); i++) {
        strs.push_back(fmtstr("{}: {}", str(self->keys[i]), str(self->values[i])));
    }

    out << "{" << join(", ", strs) << "}";
    return false;
}

ReturnType Printer::matchvalue(MatchValue const* self, int depth, std::ostream& out, int level) {
    exec(self->value, depth, out, level);
    return false;
}

ReturnType
Printer::matchsingleton(MatchSingleton const* self, int depth, std::ostream& out, int level) {
    out << self->value;
    return false;
}

ReturnType
Printer::matchsequence(MatchSequence const* self, int depth, std::ostream& out, int level) {
    auto result = join(", ", self->patterns);
    out << "[" << result << "]";
    return false;
}

ReturnType
Printer::matchmapping(MatchMapping const* self, int depth, std::ostream& out, int level) {
    Array<String> strs;
    strs.reserve(self->keys.size());

    for (int i = 0; i < self->keys.size(); i++) {
        strs.push_back(fmtstr("{}: {}", str(self->keys[i]), str(self->patterns[i])));
    }

    String remains;
    if (self->rest.has_value()) {
        StringStream ss;
        ss << ", **" << self->rest.value();
        remains = ss.str();
    }

    out << "{" << join(", ", strs) << remains << "}";
    return false;
}

ReturnType Printer::matchclass(MatchClass const* self, int depth, std::ostream& out, int level) {
    exec(self->cls, depth, out, level);
    out << "(" << join(", ", self->patterns);

    if (!self->patterns.empty() && !self->kwd_attrs.empty()) {
        out << ", ";
    }

    Array<String> kwdpat;
    kwdpat.reserve(self->kwd_attrs.size());

    for (int i = 0; i < self->kwd_attrs.size(); i++) {
        kwdpat.push_back(fmtstr("{}={}", self->kwd_attrs[i], str(self->kwd_patterns[i])));
    }

    out << join(", ", kwdpat);
    out << ")";
    return false;
}

ReturnType Printer::matchstar(MatchStar const* self, int depth, std::ostream& out, int level) {
    out << "*";

    if (self->name.has_value()) {
        out << self->name.value();
    }
    return false;
}

ReturnType Printer::matchas(MatchAs const* self, int depth, std::ostream& out, int level) {
    
    if (self->pattern.has_value()) {
        exec(self->pattern.value(), depth, out, level);

        if (self->name.has_value()) {
            out << " as " << self->name.value();
        }
    } else if (self->name.has_value()) {
        out << self->name.value();
    }

    return false;
}

ReturnType Printer::matchor(MatchOr const* self, int depth, std::ostream& out, int level) {
    out << join(" | ", self->patterns);
    return false;
}

ReturnType Printer::ifstmt(If const* self, int depth, std::ostream& out, int level) {
    out << "if ";
    exec(self->test, depth, out, level);
    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << "\n";
    print_body(self->body, depth, out, level + 1);

    for (int i = 0; i < self->tests.size(); i++) {
        auto& eliftest = self->tests[i];
        auto& elifbody = self->bodies[i];

        out << "\n" << indent(level) << "elif ";

        exec(eliftest, depth, out, level);
        out << ":";
        maybe_inline_comment(self->tests_comment[i], depth, out, level, LOC);
        out << "\n";
        print_body(elifbody, depth, out, level + 1);
    }

    if (!self->orelse.empty()) {
        out << "\n" << indent(level) << "else:";
        maybe_inline_comment(self->else_comment, depth, out, level, LOC);
        out << "\n";

        print_body(self->orelse, depth, out, level + 1);
    }
    return false;
}

ReturnType Printer::match(Match const* self, int depth, std::ostream& out, int level) {
    out << "match ";
    exec(self->subject, depth, out, level);
    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << "\n";

    int i = 0;
    for (auto const& case_: self->cases) {
        matchcase(case_, depth, out, level + 1);

        if (i + 1 < self->cases.size()) {
            out << '\n';
        }

        i += 1;
    }
    return false;
}

ReturnType Printer::lambda(Lambda const* self, int depth, std::ostream& out, int level) {
    out << "lambda ";
    arguments(self->args, depth, out, 0);
    out << ": ";
    exec(self->body, depth, out, level);
    return false;
}

ReturnType Printer::ifexp(IfExp const* self, int depth, std::ostream& out, int level) {
    exec(self->body, depth, out, level);
    out << " if ";
    exec(self->test, depth, out, level);
    out << " else ";
    exec(self->orelse, depth, out, level);
    return false;
}

ReturnType Printer::listcomp(ListComp const* self, int depth, std::ostream& out, int level) {
    out << "[";
    exec(self->elt, depth, out, level);

    out << join(" ", self->generators);

    out << "]";
    return false;
}

ReturnType Printer::setcomp(SetComp const* self, int depth, std::ostream& out, int level) {
    out << "{";
    exec(self->elt, depth, out, level);

    out << join(" ", self->generators);

    out << "}";
    return false;
}

ReturnType
Printer::generateexpr(GeneratorExp const* self, int depth, std::ostream& out, int level) {
    out << "(";
    exec(self->elt, depth, out, level);

    out << join(" ", self->generators);

    out << ")";
    return false;
}

ReturnType Printer::dictcomp(DictComp const* self, int depth, std::ostream& out, int level) {
    out << "{";
    exec(self->key, depth, out, level);
    out << ": ";
    exec(self->value, depth, out, level);

    out << join(" ", self->generators);
    out << "}";
    return false;
}

ReturnType Printer::await(Await const* self, int depth, std::ostream& out, int level) {
    out << "await ";
    exec(self->value, depth, out, level);
    return false;
}

ReturnType Printer::yield(Yield const* self, int depth, std::ostream& out, int level) {
    out << "yield";
    if (self->value.has_value()) {
        out << " ";
        exec(self->value.value(), depth, out, level);
    }
    return false;
}

ReturnType Printer::yieldfrom(YieldFrom const* self, int depth, std::ostream& out, int level) {
    out << "yield from ";
    exec(self->value, depth, out, level);
    return false;
}

ReturnType Printer::call(Call const* self, int depth, std::ostream& out, int level) {

    // StringStream fname = fmt(self->func, depth, level);
    // out << fname.str() << "(";

    exec(self->func, depth, out, level);
    out << "(";
    int k = 0;

    for (int i = 0; i < self->args.size(); i++) {
        if (k > 0) {
            out << ", ";
        }
        exec(self->args[i], depth, out, level);
        k += 1;
    }


    for (int i = 0; i < self->varargs.size(); i++) {
        if (k > 0) {
            out << ", ";
        }
        exec(self->varargs[i], depth, out, level);
        k += 1;
    }


    for (int i = 0; i < self->keywords.size(); i++) {
        if (k > 0) {
            out << ", ";
        }

        auto const& key = self->keywords[i];

        out << self->keywords[i].arg;
        out << "=";

        exec(key.value, depth, out, level);
        k += 1;
    }

    out << ")";
    return false;
}

ReturnType Printer::constant(Constant const* self, int depth, std::ostream& out, int level) {
    out << self->value;
    return false;
}

Printer::ExprRet
Printer::placeholder(Placeholder_t* self, int depth, std::ostream& out, int level) {
    return false;
}

ReturnType Printer::namedexpr(NamedExpr const* self, int depth, std::ostream& out, int level) {
    exec(self->target, depth, out, level);
    out << " := ";
    exec(self->value, depth, out, level);
    return false;
}

ReturnType Printer::classdef(ClassDef const* self, int depth, std::ostream& out, int level) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out << indent(level);
        }

        out << "@";
        exec(decorator.expr, depth, out, level);
        maybe_inline_comment(decorator.comment, depth, out, level, LOC);
        out << "\n";
        k += 1;
    }

    if (!self->decorator_list.empty()) {
        out << indent(level);
    }

    out << "class " << self->name;
    if (self->bases.size() + size_t(!self->keywords.empty())) {
        out << '(';
    }

    out << join<ExprNode*>(", ", self->bases);

    if (!self->bases.empty() && !self->keywords.empty()) {
        out << ", ";
    }

    Array<String> kwd;
    kwd.reserve(self->keywords.size());

    for (auto const& kw: self->keywords) {
        kwd.push_back(fmtstr("{}={}", str(kw.arg), str(kw.value)));
    }

    out << join(", ", kwd);

    if (self->bases.size() + self->keywords.size() > 0) {
        out << ')';
    }

    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << "\n";

    if (self->docstring.has_value()) {
        Docstring const& doc = self->docstring.value();
        out << indent(level + 1) << "\"\"\"" << doc.docstring << "\"\"\"";

        maybe_inline_comment(doc.comment, depth, out, level, LOC);
        out << "\n";
    }

    bool assign = false;
    k           = 0;
    for (auto const& stmt: self->body) {

        assign = stmt->kind == NodeKind::Assign || stmt->kind == NodeKind::AnnAssign ||
                 stmt->kind == NodeKind::Pass;

        // print an extra line before if not an attribute
        if (k > 0 && !assign) {
            out << "\n";
        }

        out << indent(level + 1);
        bool printed_new_line = exec(stmt, depth, out, level + 1);

        if (stmt->is_one_line()) {
            maybe_inline_comment(stmt->comment, depth, out, level, LOC);
        }
        if (!printed_new_line) {
            if (k + 1 < self->body.size()) {
                out << "\n";
            }
        }
        k += 1;
    }
    return false;
}

ReturnType Printer::functiondef(FunctionDef const* self, int depth, std::ostream& out, int level) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out << indent(level);
        }

        out << "@";
        exec(decorator.expr, depth, out, level);
        maybe_inline_comment(decorator.comment, depth, out, level, LOC);
        out << "\n";
        k += 1;
    }

    if (!self->decorator_list.empty()) {
        out << indent(level);
    }

    out << "def " << self->name << "(";
    arguments(self->args, depth, out, level);
    out << ")";

    if (self->returns.has_value()) {
        out << " -> ";
        exec(self->returns.value(), depth, out, level);
    }

    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << "\n";

    if (self->docstring.has_value()) {
        Docstring const& doc = self->docstring.value();
        out << indent(level + 1) << "\"\"\"" << doc.docstring << "\"\"\"";

        maybe_inline_comment(doc.comment, depth, out, level, LOC);
        out << "\n";
    }

    print_body(self->body, depth, out, level + 1, true);

    out << "\n";
    return false;
}

ReturnType Printer::inlinestmt(Inline const* self, int depth, std::ostream& out, int level) {
    out << indent(level);

    int k = 0;
    for (auto const& stmt: self->body) {
        exec(stmt, depth, out, level);

        if (k + 1 < self->body.size()) {
            out << "; ";
        }

        k += 1;
    }

    return false;
}

ReturnType Printer::forstmt(For const* self, int depth, std::ostream& out, int level) {
    out << "for ";
    exec(self->target, depth, out, -1);
    out << " in ";
    exec(self->iter, depth, out, -1);
    out << ":";

    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << "\n";

    print_body(self->body, depth, out, level + 1);

    if (!self->orelse.empty()) {
        out << '\n';
        out << indent(level) << "else:";
        maybe_inline_comment(self->else_comment, depth, out, level, LOC);
        out << "\n";
        print_body(self->orelse, depth, out, level + 1);
    }

    return false;
}

ReturnType Printer::trystmt(Try const* self, int depth, std::ostream& out, int level) {
    out << "try:";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << "\n";

    print_body(self->body, depth, out, level + 1);

    for (auto const& handler: self->handlers) {
        excepthandler(handler, depth, out, level);
    }

    if (!self->orelse.empty()) {
        out << "\n" << indent(level) << "else:";
        maybe_inline_comment(self->else_comment, depth, out, level, LOC);
        out << "\n";
        print_body(self->orelse, depth, out, level + 1);
    }

    if (!self->finalbody.empty()) {
        out << "\n" << indent(level) << "finally:";
        maybe_inline_comment(self->finally_comment, depth, out, level, LOC);
        out << "\n";
        print_body(self->finalbody, depth, out, level + 1);
    }

    return false;
}

ReturnType Printer::compare(Compare const* self, int depth, std::ostream& out, int level) {
    exec(self->left, depth, out, level);
    int n = int(self->comparators.size());

    for (int i = 0; i < self->ops.size(); i++) {
        print_op(out, self->ops[i]);

        // this can happen when the user does not finish writing the expression
        if (i < n) {
            exec(self->comparators[i], depth, out, level);
        }
    }

    return false;
}

ReturnType Printer::binop(BinOp const* self, int depth, std::ostream& out, int level) {
    auto sself   = get_precedence(self);
    auto lhspred = get_precedence(self->left) < sself;
    auto rhspred = get_precedence(self->right) < sself;

    if (lhspred) {
        out << '(';
    }
    exec(self->left, depth, out, level);
    if (lhspred) {
        out << ')';
    }

    print_op(out, self->op, false);

    if (rhspred) {
        out << '(';
    }
    exec(self->right, depth, out, level);
    if (rhspred) {
        out << ')';
    }

    return false;
}

ReturnType
Printer::invalidstmt(InvalidStatement const* self, int depth, std::ostream& out, int level) {
    //
    if (!self->tokens.empty()) {
        Unlex unlex;
        unlex.format(out, self->tokens);
    }
    return false;
}

ReturnType Printer::boolop(BoolOp const* self, int depth, std::ostream& out, int level) {

    int m = self->opcount + 1;

    int n = int(self->values.size());
    for (int i = 0; i < m; i++) {

        if (i < n) {
            exec(self->values[i], depth, out, level);
        }

        if (i < m - 1) {
            print_op(out, self->op);
        }
    }

    return false;
}

ReturnType Printer::unaryop(UnaryOp const* self, int depth, std::ostream& out, int level) {
    print_op(out, self->op);
    out << " ";
    exec(self->operand, depth, out, level);

    return false;
}

ReturnType Printer::whilestmt(While const* self, int depth, std::ostream& out, int level) {
    out << "while ";
    exec(self->test, depth, out, level);
    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << "\n";
    print_body(self->body, depth, out, level + 1);

    if (!self->orelse.empty()) {
        out << '\n' << indent(level) << "else:";
        maybe_inline_comment(self->else_comment, depth, out, level, LOC);
        out << "\n";
        print_body(self->orelse, depth, out, level + 1);
    }

    return false;
}

ReturnType Printer::returnstmt(Return const* self, int depth, std::ostream& out, int level) {
    out << "return ";

    if (self->value.has_value()) {
        exec(self->value.value(), depth, out, -1);
    }

    return false;
}

ReturnType Printer::deletestmt(Delete const* self, int depth, std::ostream& out, int level) {
    out << "del ";

    for (int i = 0; i < self->targets.size(); i++) {
        exec(self->targets[i], depth, out, level);

        if (i < self->targets.size() - 1)
            out << ", ";
    }

    return false;
}

ReturnType Printer::augassign(AugAssign const* self, int depth, std::ostream& out, int level) {
    exec(self->target, depth, out, -1);
    print_op(out, self->op, true);
    out << "= ";
    exec(self->value, depth, out, -1);
    return false;
}

ReturnType Printer::assign(Assign const* self, int depth, std::ostream& out, int level) {
    exec(self->targets[0], depth, out, -1);
    out << " = ";
    exec(self->value, depth, out, -1);
    return false;
}

ReturnType Printer::annassign(AnnAssign const* self, int depth, std::ostream& out, int level) {
    exec(self->target, depth, out, level);
    out << ": ";

    exec(self->annotation, depth, out, level);
    if (self->value.has_value()) {
        out << " = ";
        exec(self->value.value(), depth, out, level);
    }

    return false;
}

ReturnType Printer::pass(Pass const* self, int depth, std::ostream& out, int level) {
    out << "pass";
    return false;
}

ReturnType Printer::breakstmt(Break const* self, int depth, std::ostream& out, int level) {
    out << "break";
    return false;
}

ReturnType Printer::continuestmt(Continue const* self, int depth, std::ostream& out, int level) {
    out << "continue";
    return false;
}

ReturnType Printer::exprstmt(Expr const* self, int depth, std::ostream& out, int level) {
    if (self->value != nullptr)
        exec(self->value, depth, out, -1);

    return false;
}

ReturnType Printer::global(Global const* self, int depth, std::ostream& out, int level) {
    out << "global " << join(", ", self->names);
    return false;
}

ReturnType Printer::nonlocal(Nonlocal const* self, int depth, std::ostream& out, int level) {
    out << "nonlocal " << join(", ", self->names);
    return false;
}

ReturnType Printer::arrow(Arrow const* self, int depth, std::ostream& out, int level) {
    out << '(' << join<ExprNode*>(", ", self->args) << ") -> ";
    out << str(self->returns);
    return false;
}

ReturnType Printer::dicttype(DictType const* self, int depth, std::ostream& out, int level) {
    out << "Dict[";
    out << str(self->key);
    out << ", ";
    out << str(self->value) << "]";
    return false;
}

ReturnType Printer::settype(SetType const* self, int depth, std::ostream& out, int level) {
    out << "Set[";
    out << str(self->value) << "]";
    return false;
}

ReturnType Printer::name(Name const* self, int depth, std::ostream& out, int level) {
    out << self->id;
    // if (self->load_id >= 0) { 
    //     int debruijn_idx = self->load_id - self->store_id;
    //     out << "[" << debruijn_idx << "]";
    // }
    return false;
}

ReturnType Printer::arraytype(ArrayType const* self, int depth, std::ostream& out, int level) {
    out << "Array[";
    out << str(self->value) << "]";
    return false;
}

ReturnType Printer::tupletype(TupleType const* self, int depth, std::ostream& out, int level) {
    out << "Tuple[";
    out << join<ExprNode*>(", ", self->types) << "]";
    return false;
}

ReturnType Printer::builtintype(BuiltinType const* self, int depth, std::ostream& out, int level) {
    out << self->name;
    return false;
}

ReturnType Printer::joinedstr(JoinedStr const* self, int depth, std::ostream& out, int level) {
    out << "f\"";

    for (auto* val: self->values) {
        if (Constant* cst = cast<Constant>(val)) {
            out << cst->value.as<String>();
        } else {
            exec(val, depth, out, level);
        }
    }

    out << '"';
    return false;
}

ReturnType
Printer::formattedvalue(FormattedValue const* self, int depth, std::ostream& out, int indent) {
    out << "{";
    exec(self->value, depth, out, indent);
    out << ":";

    for (auto* val: self->format_spec->values) {
        if (Constant* cst = cast<Constant>(val)) {
            out << cst->value.as<String>();
        } else {
            out << "{";
            exec(val, depth, out, indent);
            out << "}";
        }
    }

    out << "}";
    return false;
}

ReturnType Printer::classtype(ClassType const* self, int depth, std::ostream& out, int level) {
    out << self->def->name;
    return false;
}

ReturnType Printer::exported(Exported const* self, int depth, std::ostream& out, int level) {
    // exec(self->node, depth, out, level);
    return false;
}

ReturnType Printer::condjump(CondJump_t* n, int depth, std::ostream& out, int level) {
    out << "condjump(";
    exec(n->condition, depth, out, level);
    out << ", ";
    out << n->then_jmp;
    out << ", ";
    out << n->else_jmp;
    out << ")";

    return false; 
}
ReturnType Printer::jump(Jump_t* n, int depth, std::ostream& out, int level) {
    out << "jump(";
    out << n->destination;
    out << ")";
    return false; 
}
ReturnType Printer::vmstmt(VMStmt_t* n, int depth, std::ostream& out, int level) {
    return exec(n->stmt, depth, out, level); 
}
ReturnType Printer::nativefunction(VMNativeFunction_t* n, int depth, std::ostream& out, int level) {
    out << "nativefunction";

    return false; 
}

// Helper
// ==================================================

// String Node::__str__() const {
//     StringStream ss;
//     if (kind <= NodeKind::Invalid) {
//         kwerror(outlog(), "Node is invalid");
//         return "<Invalid>";
//     }
//     Printer p;
//     p.Super::exec<bool>(this, 0, ss, 0);
//     return ss.str();
// }

// void Node::print(std::ostream &out, int indent) const {
//     out << "<not-implemented:";
//     out << str(kind);
//     out << ">";
// }

void print_op(std::ostream& out, BoolOperator op) {
    // clang-format off
    switch (op) {
    case BoolOperator::And:   out << " and "; return;
    case BoolOperator::Or:    out << " or " ; return;
    case BoolOperator::None:  out << " <Bool:None> " ; return;
    }
    // clang-format on
}

void print_op(std::ostream& out, BinaryOperator op, bool aug) {
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

void print_op(std::ostream& out, CmpOperator op) {
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

void print_op(std::ostream& out, UnaryOperator op) {
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

void comprehension(Printer& p, Comprehension const& self, int depth, std::ostream& out, int level) {
    out << " for ";
    p.exec(self.target, depth, out, level);
    out << " in ";
    p.exec(self.iter, depth, out, level);

    for (auto* expr: self.ifs) {
        out << " if ";
        p.exec(expr, depth, out, level);
    }
}

void Printer::keyword(Keyword const& self, int depth, std::ostream& out, int level) {
    out << self.arg;
    if (self.value != nullptr) {
        out << " = ";
        exec(self.value, depth, out, level);
    }
}

void Printer::alias(Alias const& self, int depth, std::ostream& out, int level) {
    out << self.name;
    if (self.asname.has_value()) {
        out << " as " << self.asname.value();
    }
}

ReturnType
Printer::functiontype(FunctionType const* self, int depth, std::ostream& out, int indent) {
    return false;
}

ReturnType Printer::expression(Expression const* self, int depth, std::ostream& out, int level) {
    return false;
}

ReturnType Printer::interactive(Interactive const* self, int depth, std::ostream& out, int level) {
    return false;
}

void Printer::withitem(WithItem const& self, int depth, std::ostream& out, int level) {
    exec(self.context_expr, depth, out, level);
    if (self.optional_vars.has_value()) {
        out << " as ";
        exec(self.optional_vars.value(), depth, out, level);
    }
}

ReturnType Printer::comment(Comment const* n, int depth, std::ostream& out, int level) {
    out << "#" << n->comment;
    return false;
}

void Printer::arguments(Arguments const& self, int depth, std::ostream& out, int level) {
    int i = 0;
    ArgumentKind prev = ArgumentKind::None;

    self.visit([&](ArgumentIter<true> const& iter){
        Arg const&      arg   = iter.arg;
        ArgumentKind    kind  = iter.kind;
        ExprNode const* value = iter.value;

        if (i != 0) {
            out << ", ";
        }

        if (prev == ArgumentKind::PosOnly && kind != ArgumentKind::PosOnly) {
            out << "/";
            i += 1;
            prev = kind;
            out << ", ";
        }

        if (kind == ArgumentKind::VarArg) {
            out << "*" << arg.arg;
            prev = kind;
            i += 1;
            return;
        }

        if (kind == ArgumentKind::KwArg) {
            out << "**" << arg.arg;
            prev = kind;
            i += 1;
            return;
        }

        out << arg.arg;
        if (arg.annotation.has_value()) {
            out << ": "; 
            exec(arg.annotation.value(), depth, out, level);
        }

        if (value) {
            if (arg.annotation.has_value()) {
                out << " = ";
            } else {
                out << "=";
            }
            exec(value, depth, out, level);
        }

        i += 1;
        prev = kind;
    });
}

int get_precedence(Node const* node) {
    if (node == nullptr) {
        return 1000;
    }
    if (node->kind == NodeKind::BinOp) {
        BinOp* op = (BinOp*)(node);
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

std::ostream& operator<<(std::ostream& out, VMNode const& obj) {
    Printer p;
    p.exec(&obj, 0, out, 0); 
    return out;
}

std::ostream& operator<<(std::ostream& out, Node const& obj) { 
    Printer p;
    p.exec<bool>(&obj, 0, out, 0); 
    return out;}
std::ostream& operator<<(std::ostream& out, ExprNode const& obj){ 
        Printer p;
    p.exec(&obj, 0, out, 0);
    return out;}
std::ostream& operator<<(std::ostream& out, Pattern const& obj){ 
    Printer p;
    p.exec(&obj, 0, out, 0);
    return out;}
std::ostream& operator<<(std::ostream& out, StmtNode const& obj){ 
        Printer p;
    p.exec(&obj, 0, out, 0);
    return out;}
std::ostream& operator<<(std::ostream& out, ModNode const& obj){
        Printer p;
    p.exec(&obj, 0, out, 0);
     return out;}

void print(Node const* obj, std::ostream& out) {
    Printer p;
    p.exec<bool>(obj, 0, out, 0);
}
void print(ExprNode const* obj, std::ostream& out) {
    Printer p;
    p.exec(obj, 0, out, 0);
}
void print(Pattern const* obj, std::ostream& out) {
    Printer p;
    p.exec(obj, 0, out, 0);
}
void print(StmtNode const* obj, std::ostream& out) {
    Printer p;
    p.exec(obj, 0, out, 0);
}
void print(ModNode const* obj, std::ostream& out) {
    Printer p;
    p.exec(obj, 0, out, 0);
}

String str(ExprNode const* obj) {
    StringStream ss;
    print(obj, ss);
    return ss.str();
}

String str(Node const* obj) {
    if (obj != nullptr) {
        StringStream ss;
        print(obj, ss);
        return ss.str();
    }
    return String("None");
}

}  // namespace lython
