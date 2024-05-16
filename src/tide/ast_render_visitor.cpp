#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

#include "lexer/lexer.h"
#include "tide/ast_input.h"
#include "tide/ast_render.h"

#include "ast/nodes.h"
#include "ast/ops.h"
#include "utilities/printing.h"
// #include "ast/values/native.h"
// #include "ast/values/object.h"
#include "ast/visitor.h"
#include "lexer/unlex.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/allocator.h"
#include "utilities/strings.h"

#include "dependencies/fmt.h"
#include "dependencies/formatter.h"

namespace lython {
bool safe_erase(String& str, int cursor) {
    if (cursor >= 0 && !str.empty()) {
        str.erase(cursor, 1);
        return true;
    }
    return false;
}
}  // namespace lython

// attr = StringRef(__str.substr(0, __str.size() - 1));
#define LY_BACKSPACE(obj, attr)                    \
    [this, obj](int cursor) -> bool {              \
        String __str  = str(attr);                 \
        this->_redraw = safe_erase(__str, cursor); \
        attr          = StringRef(__str);          \
        return this->_redraw;                      \
    }

// __str.push_back(c);
#define LY_INPUT(obj, attr)                           \
    [this, obj](int cursor, unsigned int c) -> bool { \
        String __str = str(attr);                     \
        __str.insert(cursor, 1, c);                   \
        attr          = StringRef(__str);             \
        this->_redraw = true;                         \
        return true;                                  \
    }

// attr = StringRef(__str.substr(0, __str.size() - 1));
#define LY_BACKSPACE_STR(obj, attr)               \
    [this, obj](int cursor) -> bool {             \
        this->_redraw = safe_erase(attr, cursor); \
        return this->_redraw;                     \
    }

// __str.push_back(c);
#define LY_INPUT_STR(obj, attr)                       \
    [this, obj](int cursor, unsigned int c) -> bool { \
        attr.insert(cursor, 1, c);                    \
        this->_redraw = true;                         \
        return true;                                  \
    }

bool ASTInputText(const char*            label,
                  const char*            hint,
                  char*                  buf,
                  int                    buf_size,
                  ImVec2&                size_arg,
                  ImGuiInputTextFlags    flags,
                  ImGuiInputTextCallback callback,
                  void*                  callback_user_data);

#define LOG(X) std::cout << X << std::endl;

namespace lython {

static special::Newline       newline;
static special::Indent        indentation;
static special::BeforeComment comment_space;

LY_ReturnType ASTRender::render_body(Array<StmtNode*>& body, int depth, bool print_last) {

    Node* parent = nullptr;

    // Module are not added to the stack
    if (stack.size() > 0) {
        parent = stack[stack.size() - 1];
    }

    int edit_entry = int(edit_order.size());
    edit_order.push_back(-1);
    int old_count = drawings.size();

    int k = 0;
    for (auto& stmt: body) {
        k += 1;

        out() << indentation;
        bool printed_new_line = false;
        run(stmt, depth);

        if (stmt->is_one_line()) {
            maybe_inline_comment(stmt->comment, depth, LOC);
        }

        // out() << newline;
        if (!printed_new_line) {
            if (k < body.size() || print_last) {
                out() << newline;
            }
        }
    }

    Group* my_group     = new_group();
    my_group->node      = parent;
    my_group->body      = &body;
    my_group->edit_id   = edit_entry;
    int new_count       = drawings.size();
    my_group->rectangle = drawings[old_count].get_mut<Drawing>()->rectangle;
    for (int i = old_count; i < new_count; i++) {
        my_group->rectangle.Add(drawings[i].get_mut<Drawing>()->rectangle);
    }

    edit_order[edit_entry] = my_group->id - 1;
    if (edit_entry != int(edit_order.size()) - 1) {
        edit_order.emplace_back(my_group->id - 1);
    }

    return true;
}

LY_ReturnType ASTRender::excepthandler(ExceptHandler& self, int depth) {
    out() << newline << indentation << special::Keyword("except ");

    if (self.type.has_value()) {
        run(self.type.value(), depth);
    }

    if (self.name.has_value()) {
        out() << special::Keyword(" as ");
        out() << self.name.value();
    }

    out() << ":";
    maybe_inline_comment(self.comment, depth, LOC);
    out() << newline;

    {
        auto _ = indent();
        return render_body(self.body, depth + 1);
    }
}

LY_ReturnType ASTRender::matchcase(MatchCase& self, int depth) {
    out() << indentation << special::Keyword("case ");
    run(self.pattern, depth);

    if (self.guard.has_value()) {
        out() << special::Keyword(" if ");
        run(self.guard.value(), depth);
    }

    out() << ":";
    maybe_inline_comment(self.comment, depth, LOC);
    out() << newline;

    {
        auto _ = indent();
        return render_body(self.body, depth + 1);
    }
}

void ASTRender::arg(Arg& self, int depth) {
    out() << self.arg;

    if (self.annotation.has_value()) {
        out() << ": ";
        run(self.annotation.value(), depth);
    }
}

LY_ReturnType ASTRender::attribute(Attribute* self, int depth) {
    run(self->value, depth);
    out() << ".";
    out() << self->attr;
    return false;
}

LY_ReturnType ASTRender::subscript(Subscript* self, int depth) {
    run(self->value, depth);
    out() << "[";
    run(self->slice, depth);
    out() << "]";
    return false;
}

LY_ReturnType ASTRender::starred(Starred* self, int depth) {
    out() << "*";
    run(self->value, depth);
    return false;
}

LY_ReturnType ASTRender::module(Module* self, int depth) { return render_body(self->body, depth); }

LY_ReturnType ASTRender::raise(Raise* self, int depth) {
    out() << special::Keyword("raise ");
    if (self->exc.has_value()) {
        run(self->exc.value(), depth);
    }

    if (self->cause.has_value()) {
        out() << special::Keyword(" from ");
        run(self->cause.value(), depth);
    }
    return false;
}

LY_ReturnType ASTRender::assertstmt(Assert* self, int depth) {
    out() << special::Keyword("assert ");
    run(self->test, depth);

    if (self->msg.has_value()) {
        out() << ", ";
        run(self->msg.value(), depth);
    }
    return false;
}

LY_ReturnType ASTRender::with(With* self, int depth) {
    out() << special::Keyword("with ");

    int i = 0;
    for (auto& item: self->items) {
        run(item.context_expr, depth);

        if (item.optional_vars.has_value()) {
            out() << special::Keyword(" as ");
            run(item.optional_vars.value(), depth);
        }

        if (i + 1 < self->items.size()) {
            out() << ", ";
        }
        i += 1;
    }
    out() << ":";
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    {
        auto _ = indent();
        render_body(self->body, depth + 1);
    }
    return false;
}

LY_ReturnType ASTRender::import(Import* self, int depth) {
    out() << special::Keyword("import ");

    int i = 0;
    for (auto& alias: self->names) {
        out() << alias.name;

        if (alias.asname.has_value()) {
            out() << special::Keyword(" as ");
            out() << alias.asname.value();
        }

        if (i + 1 < self->names.size()) {
            out() << ", ";
        }
        i += 1;
    }
    return false;
}

LY_ReturnType ASTRender::importfrom(ImportFrom* self, int depth) {
    out() << special::Keyword("from ");
    if (self->module.has_value()) {
        out() << self->module.value();
    }
    out() << special::Keyword(" import ");

    int i = 0;
    for (auto& alias: self->names) {
        out() << alias.name;

        if (alias.asname.has_value()) {
            out() << special::Keyword(" as ");
            out() << alias.asname.value();
        }

        if (i + 1 < self->names.size()) {
            out() << ", ";
        }
        i += 1;
    }
    return false;
}

LY_ReturnType ASTRender::slice(Slice* self, int depth) {
    if (self->lower.has_value()) {
        run(self->lower.value(), depth);
    }

    out() << ":";

    if (self->upper.has_value()) {
        run(self->upper.value(), depth);
    }

    if (self->step.has_value()) {
        out() << ":";
        run(self->step.value(), depth);
    }
    return false;
}

LY_ReturnType ASTRender::tupleexpr(TupleExpr* self, int depth) {
    if (level == -1) {
        out() << join<ExprNode*>(", ", self->elts);
    } else {
        out() << "(" << join<ExprNode*>(", ", self->elts) << ")";
    }
    return false;
}

LY_ReturnType ASTRender::listexpr(ListExpr* self, int depth) {
    out() << "[" << join<ExprNode*>(", ", self->elts) << "]";
    return false;
}

LY_ReturnType ASTRender::setexpr(SetExpr* self, int depth) {
    out() << "{" << join<ExprNode*>(", ", self->elts) << "}";
    return false;
}

LY_ReturnType ASTRender::dictexpr(DictExpr* self, int depth) {
    Array<String> strs;
    strs.reserve(self->keys.size());

    for (int i = 0; i < self->keys.size(); i++) {
        strs.push_back(fmtstr("{}: {}", str(self->keys[i]), str(self->values[i])));
    }

    out() << "{" << join(", ", strs) << "}";
    return false;
}

LY_ReturnType ASTRender::matchvalue(MatchValue* self, int depth) {
    run(self->value, depth);
    return false;
}

LY_ReturnType ASTRender::matchsingleton(MatchSingleton* self, int depth) {
    StringStream ss;
    ss << self->value;
    out() << ss.str();
    return false;
}

LY_ReturnType ASTRender::matchsequence(MatchSequence* self, int depth) {
    auto result = join(", ", self->patterns);
    out() << "[" << result << "]";
    return false;
}

LY_ReturnType ASTRender::matchmapping(MatchMapping* self, int depth) {
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

    out() << "{" << join(", ", strs) << remains << "}";
    return false;
}

LY_ReturnType ASTRender::matchclass(MatchClass* self, int depth) {
    run(self->cls, depth);
    out() << "(" << join(", ", self->patterns);

    if (!self->patterns.empty() && !self->kwd_attrs.empty()) {
        out() << ", ";
    }

    Array<String> kwdpat;
    kwdpat.reserve(self->kwd_attrs.size());

    for (int i = 0; i < self->kwd_attrs.size(); i++) {
        kwdpat.push_back(fmtstr("{}={}", self->kwd_attrs[i], str(self->kwd_patterns[i])));
    }

    out() << join(", ", kwdpat);
    out() << ")";
    return false;
}

LY_ReturnType ASTRender::matchstar(MatchStar* self, int depth) {
    out() << "*";

    if (self->name.has_value()) {
        out() << self->name.value();
    }
    return false;
}

LY_ReturnType ASTRender::matchas(MatchAs* self, int depth) {
    if (self->pattern.has_value()) {
        run(self->pattern.value(), depth);
    }

    if (self->name.has_value()) {
        out() << special::Keyword(" as ") << self->name.value();
    }
    return false;
}

LY_ReturnType ASTRender::matchor(MatchOr* self, int depth) {
    out() << join(" | ", self->patterns);
    return false;
}

LY_ReturnType ASTRender::ifstmt(If* self, int depth) {
    out() << special::Keyword("if ");
    run(self->test, depth);
    out() << ":";
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    {
        auto _ = indent();
        render_body(self->body, depth + 1);
    }

    for (int i = 0; i < self->tests.size(); i++) {
        auto& eliftest = self->tests[i];
        auto& elifbody = self->bodies[i];

        out() << newline << indentation << special::Keyword("elif ");

        run(eliftest, depth);
        out() << ":";
        maybe_inline_comment(self->tests_comment[i], depth, LOC);
        out() << newline;
        {
            auto _ = indent();
            render_body(elifbody, depth + 1);
        }
    }

    if (!self->orelse.empty()) {
        out() << newline << indentation << special::Keyword("else:");
        maybe_inline_comment(self->else_comment, depth, LOC);
        out() << newline;

        {
            auto _ = indent();
            render_body(self->orelse, depth + 1);
        }
    }
    return false;
}

LY_ReturnType ASTRender::match(Match* self, int depth) {
    out() << special::Keyword("match ");
    run(self->subject, depth);
    out() << ":";
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    int i = 0;
    for (auto& case_: self->cases) {
        matchcase(case_, depth + 1);

        if (i + 1 < self->cases.size()) {
            out() << newline;
        }

        i += 1;
    }
    return false;
}

LY_ReturnType ASTRender::lambda(Lambda* self, int depth) {
    out() << special::Keyword("lambda ");
    arguments(self->args, depth);
    out() << ": ";
    run(self->body, depth);
    return false;
}

LY_ReturnType ASTRender::ifexp(IfExp* self, int depth) {
    run(self->body, depth);
    out() << special::Keyword(" if ");
    run(self->test, depth);
    out() << special::Keyword(" else ");
    run(self->orelse, depth);
    return false;
}

void ASTRender::comprehension(Comprehension& self, int depth) {
    out() << special::Keyword(" for ");
    run(self.target, depth);
    out() << special::Keyword(" in ");
    run(self.iter, depth);

    for (auto* expr: self.ifs) {
        out() << special::Keyword(" if ");
        run(expr, depth);
    }
}

void ASTRender::comprehensions(Array<Comprehension>& self, int depth) {
    for (Comprehension& comp: self) {
        comprehension(comp, depth);
    }
}

LY_ReturnType ASTRender::listcomp(ListComp* self, int depth) {
    out() << "[";
    run(self->elt, depth);

    comprehensions(self->generators, depth);

    out() << "]";
    return false;
}

LY_ReturnType ASTRender::setcomp(SetComp* self, int depth) {
    out() << "{";
    run(self->elt, depth);

    comprehensions(self->generators, depth);

    out() << "}";
    return false;
}

LY_ReturnType ASTRender::generateexpr(GeneratorExp* self, int depth) {
    out() << "(";
    run(self->elt, depth);

    comprehensions(self->generators, depth);

    out() << ")";
    return false;
}

LY_ReturnType ASTRender::dictcomp(DictComp* self, int depth) {
    out() << "{";
    run(self->key, depth);
    out() << ": ";
    run(self->value, depth);

    comprehensions(self->generators, depth);
    out() << "}";
    return false;
}

LY_ReturnType ASTRender::await(Await* self, int depth) {
    out() << special::Keyword("await ");
    run(self->value, depth);
    return false;
}

LY_ReturnType ASTRender::yield(Yield* self, int depth) {
    out() << special::Keyword("yield");
    if (self->value.has_value()) {
        out() << " ";
        run(self->value.value(), depth);
    }
    return false;
}

LY_ReturnType ASTRender::yieldfrom(YieldFrom* self, int depth) {
    out() << special::Keyword("yield from ");
    run(self->value, depth);
    return false;
}

LY_ReturnType ASTRender::call(Call* self, int depth) {

    // StringStream fname = fmt(self->func, depth, level);
    // out() << fname.str() << "(";

    run(self->func, depth);
    out() << "(";

    for (int i = 0; i < self->args.size(); i++) {
        run(self->args[i], depth);

        if (i < self->args.size() - 1 || !self->keywords.empty())
            out() << ", ";
    }

    for (int i = 0; i < self->keywords.size(); i++) {
        auto& key = self->keywords[i];

        out() << self->keywords[i].arg;
        out() << "=";

        run(key.value, depth);

        if (i < self->keywords.size() - 1)
            out() << ", ";
    }

    out() << ")";
    return false;
}

LY_ReturnType ASTRender::constant(Constant* self, int depth) {
    StringStream ss;
    ss << self->value;

    // bit more tricky here
    // we need to parse the value on change
    // and the value might not be correct so we need to keep
    // a input buffer
    auto config      = special::Editable(ss.str(), self);
    config.input     = [](int cursor, unsigned int c) -> bool { return false; };
    config.backspace = [](int cursor) -> bool { return false; };
    out() << config;
    return false;
}

ASTRender::ExprRet ASTRender::placeholder(Placeholder_t* self, int depth) { return false; }

LY_ReturnType ASTRender::namedexpr(NamedExpr* self, int depth) {
    run(self->target, depth);
    out() << " := ";
    run(self->value, depth);
    return false;
}

LY_ReturnType ASTRender::classdef(ClassDef* self, int depth) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out() << indentation;
        }

        out() << "@";
        run(decorator.expr, depth);
        maybe_inline_comment(decorator.comment, depth, LOC);
        out() << newline;
        k += 1;
    }

    if (!self->decorator_list.empty()) {
        out() << indentation;
    }

    out() << special::Keyword("class ") << self->name;
    if (self->bases.size() + size_t(!self->keywords.empty())) {
        out() << '(';
    }

    out() << join<ExprNode*>(", ", self->bases);

    if (!self->bases.empty() && !self->keywords.empty()) {
        out() << ", ";
    }

    Array<String> kwd;
    kwd.reserve(self->keywords.size());

    for (auto& kw: self->keywords) {
        kwd.push_back(fmtstr("{}={}", str(kw.arg), str(kw.value)));
    }

    out() << join(", ", kwd);

    if (self->bases.size() + self->keywords.size() > 0) {
        out() << ')';
    }

    out() << ":";
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    if (self->docstring.has_value()) {
        Docstring& doc = self->docstring.value();

        out() << indentation << "\"\"\"" << doc.docstring << "\"\"\"";

        maybe_inline_comment(doc.comment, depth, LOC);
        out() << newline;
    }

    bool assign = false;
    k           = 0;
    for (auto& stmt: self->body) {

        assign = stmt->kind == NodeKind::Assign || stmt->kind == NodeKind::AnnAssign ||
                 stmt->kind == NodeKind::Pass;

        // print an extra line before if not an attribute
        if (k > 0 && !assign) {
            out() << newline;
        }

        out() << indentation;  // indent(level + 1);
        bool printed_new_line = false;
        run(stmt, depth + 1);

        if (stmt->is_one_line()) {
            maybe_inline_comment(stmt->comment, depth, LOC);
        }
        if (!printed_new_line) {
            if (k + 1 < self->body.size()) {
                out() << newline;
            }
        }
        k += 1;
    }
    return false;
}

LY_ReturnType ASTRender::functiondef(FunctionDef* self, int depth) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out() << indentation;
        }

        out() << "@";
        run(decorator.expr, depth);
        maybe_inline_comment(decorator.comment, depth, LOC);
        out() << newline;
        k += 1;
    }

    if (!self->decorator_list.empty()) {
        out() << indentation;
    }

    auto config = special::Editable(self->name ? self->name : StringRef("<function>"), (Node*)self);
    config.backspace = LY_BACKSPACE(self, self->name);
    config.input     = LY_INPUT(self, self->name);

    out() << special::Keyword("def ") << config << "(";
    arguments(self->args, depth);
    out() << ")";

    if (self->returns.has_value()) {
        out() << " -> ";
        {
            auto _ = type();
            run(self->returns.value(), depth);
        }
    }

    out() << ":";
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    if (self->docstring.has_value()) {
        Docstring& doc    = self->docstring.value();
        auto       config = special::Docstring(doc.docstring);

        config.input     = LY_INPUT_STR(&doc, doc.docstring);
        config.backspace = LY_BACKSPACE_STR(&doc, doc.docstring);

        {
            auto _ = indent();
            out() << indentation << config;
        }

        maybe_inline_comment(doc.comment, depth, LOC);
        out() << newline;
    }

    {
        auto _ = indent();
        render_body(self->body, depth + 1, true);
    }

    out() << newline;
    return false;
}

LY_ReturnType ASTRender::inlinestmt(Inline* self, int depth) {
    out() << indentation;

    int k = 0;
    for (auto& stmt: self->body) {
        run(stmt, depth);

        if (k + 1 < self->body.size()) {
            out() << "; ";
        }

        k += 1;
    }

    return false;
}

LY_ReturnType ASTRender::forstmt(For* self, int depth) {
    out() << special::Keyword("for ");
    run(self->target, depth);
    out() << special::Keyword(" in ");
    run(self->iter, depth);
    out() << ":";

    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    {
        auto _ = indent();
        render_body(self->body, depth + 1);
    }

    if (!self->orelse.empty()) {
        out() << newline;
        out() << indentation << special::Keyword("else:");
        maybe_inline_comment(self->else_comment, depth, LOC);
        out() << newline;

        {
            auto _ = indent();
            render_body(self->orelse, depth + 1);
        }
    }

    return false;
}

LY_ReturnType ASTRender::trystmt(Try* self, int depth) {
    out() << special::Keyword("try:");
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    {
        auto _ = indent();
        render_body(self->body, depth + 1);
    }

    for (auto& handler: self->handlers) {
        excepthandler(handler, depth);
    }

    if (!self->orelse.empty()) {
        out() << newline << indentation << special::Keyword("else:");
        maybe_inline_comment(self->else_comment, depth, LOC);
        out() << newline;

        {
            auto _ = indent();
            render_body(self->orelse, depth + 1);
        }
    }

    if (!self->finalbody.empty()) {
        out() << newline << indentation << special::Keyword("finally:");
        maybe_inline_comment(self->finally_comment, depth, LOC);
        out() << newline;

        {
            auto _ = indent();
            render_body(self->finalbody, depth + 1);
        }
    }

    return false;
}

LY_ReturnType ASTRender::compare(Compare* self, int depth) {
    run(self->left, depth);
    int n = int(self->comparators.size());

    for (int i = 0; i < self->ops.size(); i++) {
        print_op(self->ops[i]);

        // this can happen when the user does not finish writing the expression
        if (i < n) {
            run(self->comparators[i], depth);
        }
    }

    return false;
}

LY_ReturnType ASTRender::binop(BinOp* self, int depth) {
    auto sself   = get_precedence(self);
    auto lhspred = get_precedence(self->left) < sself;
    auto rhspred = get_precedence(self->right) < sself;

    if (lhspred) {
        out() << '(';
    }
    run(self->left, depth);
    if (lhspred) {
        out() << ')';
    }

    print_op(self, self->op, false);

    if (rhspred) {
        out() << '(';
    }
    run(self->right, depth);
    if (rhspred) {
        out() << ')';
    }

    return false;
}

LY_ReturnType ASTRender::invalidstmt(InvalidStatement* self, int depth) {
    //
    if (!self->tokens.empty()) {
        Unlex        unlex;
        StringStream ss;
        unlex.format(ss, self->tokens);
        out() << ss.str();
    }
    return false;
}

LY_ReturnType ASTRender::boolop(BoolOp* self, int depth) {

    int m = self->opcount + 1;

    int n = int(self->values.size());
    for (int i = 0; i < m; i++) {

        if (i < n) {
            run(self->values[i], depth);
        }

        if (i < m - 1) {
            print_op(self->op);
        }
    }

    return false;
}

LY_ReturnType ASTRender::unaryop(UnaryOp* self, int depth) {
    print_op(self->op);
    out() << " ";
    run(self->operand, depth);

    return false;
}

LY_ReturnType ASTRender::whilestmt(While* self, int depth) {
    out() << special::Keyword("while ");
    run(self->test, depth);
    out() << ":";
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;
    {
        auto _ = indent();
        render_body(self->body, depth + 1);
    }

    if (!self->orelse.empty()) {
        out() << newline << indentation << special::Keyword("else:");
        maybe_inline_comment(self->else_comment, depth, LOC);
        out() << newline;

        {
            auto _ = indent();
            render_body(self->orelse, depth + 1);
        }
    }

    return false;
}

LY_ReturnType ASTRender::returnstmt(Return* self, int depth) {
    out() << special::Keyword("return ");

    if (self->value.has_value()) {
        run(self->value.value(), depth);
    }

    return false;
}

LY_ReturnType ASTRender::deletestmt(Delete* self, int depth) {
    out() << special::Keyword("del ");

    for (int i = 0; i < self->targets.size(); i++) {
        run(self->targets[i], depth);

        if (i < self->targets.size() - 1)
            out() << ", ";
    }

    return false;
}

LY_ReturnType ASTRender::augassign(AugAssign* self, int depth) {
    run(self->target, depth);
    print_op(self, self->op, true);
    out() << "= ";
    run(self->value, depth);
    return false;
}

LY_ReturnType ASTRender::assign(Assign* self, int depth) {
    run(self->targets[0], depth);
    out() << " = ";
    run(self->value, depth);
    return false;
}

LY_ReturnType ASTRender::annassign(AnnAssign* self, int depth) {
    run(self->target, depth);
    out() << ": ";

    run(self->annotation, depth);
    if (self->value.has_value()) {
        out() << " = ";
        run(self->value.value(), depth);
    }

    return false;
}

LY_ReturnType ASTRender::pass(Pass* self, int depth) {
    out() << special::Keyword("pass");
    return false;
}

LY_ReturnType ASTRender::breakstmt(Break* self, int depth) {
    out() << special::Keyword("break");
    return false;
}

LY_ReturnType ASTRender::continuestmt(Continue* self, int depth) {
    out() << special::Keyword("continue");
    return false;
}

LY_ReturnType ASTRender::exprstmt(Expr* self, int depth) {
    if (self->value != nullptr)
        run(self->value, depth);

    return false;
}

LY_ReturnType ASTRender::global(Global* self, int depth) {
    out() << special::Keyword("global ") << join(", ", self->names);
    return false;
}

LY_ReturnType ASTRender::nonlocal(Nonlocal* self, int depth) {
    out() << special::Keyword("nonlocal ") << join(", ", self->names);
    return false;
}

LY_ReturnType ASTRender::arrow(Arrow* self, int depth) {
    out() << '(' << join<ExprNode*>(", ", self->args) << ") -> ";
    out() << str(self->returns);
    return false;
}

LY_ReturnType ASTRender::dicttype(DictType* self, int depth) {
    out() << "Dict[";
    out() << str(self->key);  // this is wrong
    out() << ", ";
    out() << str(self->value) << "]";
    return false;
}

LY_ReturnType ASTRender::settype(SetType* self, int depth) {
    out() << "Set[";
    out() << str(self->value) << "]";
    return false;
}

LY_ReturnType ASTRender::name(Name* self, int depth) {
    auto config      = special::Editable(self->id, self);
    config.backspace = LY_BACKSPACE(self, self->id);
    config.input     = LY_INPUT(self, self->id);
    out() << config;
    return false;
}

LY_ReturnType ASTRender::arraytype(ArrayType* self, int depth) {
    out() << "Array[";
    out() << str(self->value) << "]";
    return false;
}

LY_ReturnType ASTRender::tupletype(TupleType* self, int depth) {
    out() << "Tuple[";
    out() << join<ExprNode*>(", ", self->types) << "]";
    return false;
}

LY_ReturnType ASTRender::builtintype(BuiltinType* self, int depth) {
    out() << self->name;
    return false;
}

LY_ReturnType ASTRender::joinedstr(JoinedStr* self, int depth) {
    out() << "f\"";

    for (auto* val: self->values) {
        if (Constant* cst = cast<Constant>(val)) {
            out() << cst->value.as<String>();
        } else {
            run(val, depth);
        }
    }

    out() << '"';
    return false;
}

LY_ReturnType ASTRender::formattedvalue(FormattedValue* self, int depth) {
    out() << "{";
    run(self->value, depth);
    out() << ":";

    for (auto* val: self->format_spec->values) {
        if (Constant* cst = cast<Constant>(val)) {
            out() << cst->value.as<String>();
        } else {
            out() << "{";
            run(val, depth);
            out() << "}";
        }
    }

    out() << "}";
    return false;
}

LY_ReturnType ASTRender::classtype(ClassType* self, int depth) {
    out() << self->def->name;
    return false;
}

LY_ReturnType ASTRender::exported(Exported* self, int depth) {
    // run(self->node, depth);
    return false;
}

// Helper
// ==================================================

void ASTRender::print_op(BoolOperator op) {
    // clang-format off
    switch (op) {
    case BoolOperator::And:   out() << special::Keyword(" and "); return;
    case BoolOperator::Or:    out() << special::Keyword(" or ") ; return;
    case BoolOperator::None:  out() << " <Bool:None> " ; return;
    }
    // clang-format on
}

OpConfig get_operator_config(String const& op) {
    for (auto const& opconf: all_operators()) {
        if (opconf.operator_name == op) {
            return opconf;
        }
    }
    return OpConfig();
}

String to_str(BinaryOperator op) {
    for (auto const& opconf: all_operators()) {
        if (opconf.binarykind == op) {
            return opconf.operator_name;
        }
    }
    return ("<Binary Operator>");
}

BinaryOperator to_binop(String const& op) {
    if (OpConfig opconf = get_operator_config(op)) {
        return opconf.binarykind;
    }
    return BinaryOperator::None;
}

void ASTRender::print_op(Node* bin, BinaryOperator op, bool aug) {
    // clang-format off
    out() << " ";

    auto iter = buffer.find(bin);
    if (iter == buffer.end()) {
        buffer[bin] = to_str(op);
    }

    auto config = special::Editable(buffer[bin].empty() ? "<binop>" : buffer[bin], bin);

    auto update_op = [this, bin]() {
        auto newop = to_binop(this->buffer[bin]);
        if (newop != BinaryOperator::None) {
            // update the operator
            if (BinOp* binop = cast<BinOp>(bin)) {
                binop->op = newop;
            }

            if (AugAssign* aug = cast<AugAssign>(bin)) {
                aug->op = newop;
            }
        }
        // here we are checking for full match
        // but we could be checking for partial match
        // and fill the auto complete

        // TODO: handle the wrong op here
        // This is a parsing/syntax error issue that needs to be shown
    };

    // we need a buffer for the input
    // we display the buffer, update the buffer
    // when the input is valid we update the tree
    config.input = [this, bin, update_op](int cursor_1, unsigned int c_1) -> bool {
        LY_INPUT(bin, this->buffer[bin])(cursor_1, c_1);    // handle the input
        update_op();                                        // update the tree
        return true;
    };
    config.backspace = [this, bin, update_op](int cursor_1) -> bool {
        bool success = LY_BACKSPACE(bin, this->buffer[bin])(cursor_1); // handle the input
        update_op();                                    // update the tree
        return success;
    };

    out() << config;

    if (!aug) {
        out() << " ";
    }
}

void ASTRender::print_op(CmpOperator op) {
    // clang-format off
    switch (op) {
    case CmpOperator::None:     out() << " <Cmp:None> "; return;
    case CmpOperator::Eq:       out() << " == ";  return;
    case CmpOperator::NotEq:    out() << " != ";  return;
    case CmpOperator::Lt:       out() << " < ";   return;
    case CmpOperator::LtE:      out() << " <= ";  return;
    case CmpOperator::Gt:       out() << " > ";   return;
    case CmpOperator::GtE:      out() << " >= ";  return;
    case CmpOperator::Is:       out() << special::Keyword(" is ");  return;
    case CmpOperator::IsNot:    out() << special::Keyword(" is not ");  return;
    case CmpOperator::In:       out() << special::Keyword(" in ");      return;
    case CmpOperator::NotIn:    out() << special::Keyword(" not in ");  return;
    }
    // clang-format on
}

void ASTRender::print_op(UnaryOperator op) {
    // clang-format off
    switch (op) {
    case UnaryOperator::None:   out() << "<Unary:None>"; return;
    case UnaryOperator::Invert: out() << "~"; return;
    case UnaryOperator::Not:    out() << "!"; return;
    case UnaryOperator::UAdd:   out() << "+"; return;
    case UnaryOperator::USub:   out() << "-"; return;
    }
    // clang-format on
}

void ASTRender::keyword(Keyword& self, int depth) {
    out() << self.arg;
    if (self.value != nullptr) {
        out() << " = ";
        run(self.value, depth);
    }
}

void ASTRender::alias(Alias& self, int depth) {
    out() << self.name;
    if (self.asname.has_value()) {
        out() << special::Keyword(" as ") << self.asname.value();
    }
}

LY_ReturnType ASTRender::functiontype(FunctionType* self, int depth) { return false; }

LY_ReturnType ASTRender::expression(Expression* self, int depth) { return false; }

LY_ReturnType ASTRender::interactive(Interactive* self, int depth) { return false; }

void ASTRender::withitem(WithItem& self, int depth) {
    run(self.context_expr, depth);
    if (self.optional_vars.has_value()) {
        out() << special::Keyword(" as ");
        run(self.optional_vars.value(), depth);
    }
}

LY_ReturnType ASTRender::comment(Comment* n, int depth) {
    auto config      = special::Editable(!n->comment.empty() ? n->comment : String("<comment>"), n);
    config.input     = LY_INPUT_STR(n, n->comment);
    config.backspace = LY_BACKSPACE_STR(n, n->comment);
    out() << "# " << config;
    return false;
}

void ASTRender::arguments(Arguments& self, int depth) {
    int i = 0;

    for (Arg& arg: self.args) {
        auto config =
            special::Editable(arg.arg ? arg.arg : StringRef("<arg name>"), stack[stack.size() - 1]);
        config.backspace = LY_BACKSPACE(&arg, arg.arg);
        config.input     = LY_INPUT(&arg, arg.arg);

        out() << config;

        if (arg.annotation.has_value()) {
            out() << ": ";

            {
                auto _ = type();
                run(arg.annotation.value(), depth);
            }
        }

        auto default_offset = self.args.size() - 1 - i;
        if (self.defaults.size() > 0 && default_offset < self.defaults.size()) {
            if (arg.annotation.has_value()) {
                out() << " = ";
            } else {
                out() << "=";
            }
            run(self.defaults[default_offset], depth);
        }

        if (i + 1 < self.args.size()) {
            out() << ", ";
        }
        i += 1;
    }

    if (self.vararg.has_value()) {
        if (self.args.size() > 0) {
            out() << ", ";
        }

        out() << "*" << self.vararg.value().arg;
    }

    if ((self.args.size() > 0 || self.vararg.has_value()) && self.kwonlyargs.size() > 0) {
        out() << ", ";
    }

    i = 0;
    for (auto& kw: self.kwonlyargs) {
        out() << kw.arg;

        if (kw.annotation.has_value()) {
            out() << ": ";
            run(kw.annotation.value(), depth);
        }

        auto default_offset = self.kwonlyargs.size() - 1 - i;
        if (self.kw_defaults.size() > 0 && default_offset < self.kw_defaults.size()) {
            if (kw.annotation.has_value()) {
                out() << " = ";
            } else {
                out() << "=";
            }
            run(self.kw_defaults[default_offset], depth);
        }

        if (i + 1 < self.kwonlyargs.size()) {
            out() << ", ";
        }
        i += 1;
    }

    if (self.kwarg.has_value()) {
        if (!self.kwonlyargs.empty()) {
            out() << ", ";
        }
        out() << "**" << self.kwarg.value().arg;
    }
}

// LY_ReturnType ASTRender::condjump(CondJump_t* n, int depth) { return false; }

}  // namespace lython
