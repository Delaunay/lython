#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

#include "tide/ast_input.h"
#include "tide/ast_render.h"

#include "ast/magic.h"
#include "ast/nodes.h"
#include "ast/ops.h"
#include "ast/values/native.h"
#include "ast/values/object.h"
#include "ast/visitor.h"
#include "dependencies/fmt.h"
#include "lexer/unlex.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/allocator.h"
#include "utilities/strings.h"

#define MAYBE_CONST 

bool ASTInputText(const char*            label,
                  const char*            hint,
                  char*                  buf,
                  int                    buf_size,
                  MAYBE_CONST ImVec2&          size_arg,
                  ImGuiInputTextFlags    flags,
                  ImGuiInputTextCallback callback,
                  void*                  callback_user_data);

#define LOG(X) std::cout << X << std::endl;

namespace lython {

static special::Newline       newline;
static special::Indent        indentation;
static special::BeforeComment comment_space;

ReturnType ASTRender::render_body(Array<StmtNode*> MAYBE_CONST& body, int depth, bool print_last) {

    Node* parent = nullptr;

    // Module are not added to the stack
    if (stack.size() > 0) {
        parent = stack[stack.size() - 1];
    }

    int edit_entry = int(edit_order.size());
    edit_order.push_back(-1);
    int old_count = drawings.size();

    int k = 0;
    for (auto MAYBE_CONST& stmt: body) {
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
    my_group->edit_id = edit_entry;
    int new_count       = drawings.size();
    my_group->rectangle = drawings[old_count].rectangle;
    for (int i = old_count; i < new_count; i++) {
        my_group->rectangle.Add(drawings[i].rectangle);
    }

    edit_order[edit_entry] = my_group->id - 1;
    if (edit_entry != int(edit_order.size()) - 1) {
        edit_order.emplace_back(my_group->id - 1);
    }

    return true;
}

ReturnType ASTRender::excepthandler(ExceptHandler MAYBE_CONST& self, int depth) {
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

ReturnType ASTRender::matchcase(MatchCase MAYBE_CONST& self, int depth) {
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

void ASTRender::arg(Arg MAYBE_CONST& self, int depth) {
    out() << self.arg;

    if (self.annotation.has_value()) {
        out() << ": ";
        run(self.annotation.value(), depth);
    }
}

ReturnType ASTRender::attribute(Attribute MAYBE_CONST* self, int depth) {
    run(self->value, depth);
    out() << ".";
    out() << self->attr;
    return false;
}

ReturnType ASTRender::subscript(Subscript MAYBE_CONST* self, int depth) {
    run(self->value, depth);
    out() << "[";
    run(self->slice, depth);
    out() << "]";
    return false;
}

ReturnType ASTRender::starred(Starred MAYBE_CONST* self, int depth) {
    out() << "*";
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::module(Module MAYBE_CONST* self, int depth) {
    return render_body(self->body, depth);
}

ReturnType ASTRender::raise(Raise MAYBE_CONST* self, int depth) {
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

ReturnType ASTRender::assertstmt(Assert MAYBE_CONST* self, int depth) {
    out() << special::Keyword("assert ");
    run(self->test, depth);

    if (self->msg.has_value()) {
        out() << ", ";
        run(self->msg.value(), depth);
    }
    return false;
}

ReturnType ASTRender::with(With MAYBE_CONST* self, int depth) {
    out() << special::Keyword("with ");

    int i = 0;
    for (auto MAYBE_CONST& item: self->items) {
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

ReturnType ASTRender::import(Import MAYBE_CONST* self, int depth) {
    out() << special::Keyword("import ");

    int i = 0;
    for (auto MAYBE_CONST& alias: self->names) {
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

ReturnType ASTRender::importfrom(ImportFrom MAYBE_CONST* self, int depth) {
    out() << special::Keyword("from ");
    if (self->module.has_value()) {
        out() << self->module.value();
    }
    out() << special::Keyword(" import ");

    int i = 0;
    for (auto MAYBE_CONST& alias: self->names) {
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

ReturnType ASTRender::slice(Slice MAYBE_CONST* self, int depth) {
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

ReturnType ASTRender::tupleexpr(TupleExpr MAYBE_CONST* self, int depth) {
    if (level == -1) {
        out() << join<ExprNode*>(", ", self->elts);
    } else {
        out() << "(" << join<ExprNode*>(", ", self->elts) << ")";
    }
    return false;
}

ReturnType ASTRender::listexpr(ListExpr MAYBE_CONST* self, int depth) {
    out() << "[" << join<ExprNode*>(", ", self->elts) << "]";
    return false;
}

ReturnType ASTRender::setexpr(SetExpr MAYBE_CONST* self, int depth) {
    out() << "{" << join<ExprNode*>(", ", self->elts) << "}";
    return false;
}

ReturnType ASTRender::dictexpr(DictExpr MAYBE_CONST* self, int depth) {
    Array<String> strs;
    strs.reserve(self->keys.size());

    for (int i = 0; i < self->keys.size(); i++) {
        strs.push_back(fmtstr("{}: {}", str(self->keys[i]), str(self->values[i])));
    }

    out() << "{" << join(", ", strs) << "}";
    return false;
}

ReturnType ASTRender::matchvalue(MatchValue MAYBE_CONST* self, int depth) {
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::matchsingleton(MatchSingleton MAYBE_CONST* self, int depth) {
    StringStream ss;
    self->value.print(ss);
    out() << ss.str();
    return false;
}

ReturnType ASTRender::matchsequence(MatchSequence MAYBE_CONST* self, int depth) {
    auto result = join(", ", self->patterns);
    out() << "[" << result << "]";
    return false;
}

ReturnType ASTRender::matchmapping(MatchMapping MAYBE_CONST* self, int depth) {
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

ReturnType ASTRender::matchclass(MatchClass MAYBE_CONST* self, int depth) {
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

ReturnType ASTRender::matchstar(MatchStar MAYBE_CONST* self, int depth) {
    out() << "*";

    if (self->name.has_value()) {
        out() << self->name.value();
    }
    return false;
}

ReturnType ASTRender::matchas(MatchAs MAYBE_CONST* self, int depth) {
    if (self->pattern.has_value()) {
        run(self->pattern.value(), depth);
    }

    if (self->name.has_value()) {
        out() << special::Keyword(" as ") << self->name.value();
    }
    return false;
}

ReturnType ASTRender::matchor(MatchOr MAYBE_CONST* self, int depth) {
    out() << join(" | ", self->patterns);
    return false;
}

ReturnType ASTRender::ifstmt(If MAYBE_CONST* self, int depth) {
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

ReturnType ASTRender::match(Match MAYBE_CONST* self, int depth) {
    out() << special::Keyword("match ");
    run(self->subject, depth);
    out() << ":";
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    int i = 0;
    for (auto MAYBE_CONST& case_: self->cases) {
        matchcase(case_, depth + 1);

        if (i + 1 < self->cases.size()) {
            out() << newline;
        }

        i += 1;
    }
    return false;
}

ReturnType ASTRender::lambda(Lambda MAYBE_CONST* self, int depth) {
    out() << special::Keyword("lambda ");
    arguments(self->args, depth);
    out() << ": ";
    run(self->body, depth);
    return false;
}

ReturnType ASTRender::ifexp(IfExp MAYBE_CONST* self, int depth) {
    run(self->body, depth);
    out() << special::Keyword(" if ");
    run(self->test, depth);
    out() << special::Keyword(" else ");
    run(self->orelse, depth);
    return false;
}

void ASTRender::comprehension(Comprehension MAYBE_CONST& self, int depth) {
    out() << special::Keyword(" for ");
    run(self.target, depth);
    out() << special::Keyword(" in ");
    run(self.iter, depth);

    for (auto* expr: self.ifs) {
        out() << special::Keyword(" if ");
        run(expr, depth);
    }
}

void ASTRender::comprehensions(Array<Comprehension> MAYBE_CONST& self, int depth) {
    for (Comprehension MAYBE_CONST& comp: self) {
        comprehension(comp, depth);
    }
}

ReturnType ASTRender::listcomp(ListComp MAYBE_CONST* self, int depth) {
    out() << "[";
    run(self->elt, depth);

    comprehensions(self->generators, depth);

    out() << "]";
    return false;
}

ReturnType ASTRender::setcomp(SetComp MAYBE_CONST* self, int depth) {
    out() << "{";
    run(self->elt, depth);

    comprehensions(self->generators, depth);

    out() << "}";
    return false;
}

ReturnType ASTRender::generateexpr(GeneratorExp MAYBE_CONST* self, int depth) {
    out() << "(";
    run(self->elt, depth);

    comprehensions(self->generators, depth);

    out() << ")";
    return false;
}

ReturnType ASTRender::dictcomp(DictComp MAYBE_CONST* self, int depth) {
    out() << "{";
    run(self->key, depth);
    out() << ": ";
    run(self->value, depth);

    comprehensions(self->generators, depth);
    out() << "}";
    return false;
}

ReturnType ASTRender::await(Await MAYBE_CONST* self, int depth) {
    out() << special::Keyword("await ");
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::yield(Yield MAYBE_CONST* self, int depth) {
    out() << special::Keyword("yield");
    if (self->value.has_value()) {
        out() << " ";
        run(self->value.value(), depth);
    }
    return false;
}

ReturnType ASTRender::yieldfrom(YieldFrom MAYBE_CONST* self, int depth) {
    out() << special::Keyword("yield from ");
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::call(Call MAYBE_CONST* self, int depth) {

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
        auto MAYBE_CONST& key = self->keywords[i];

        out() << self->keywords[i].arg;
        out() << "=";

        run(key.value, depth);

        if (i < self->keywords.size() - 1)
            out() << ", ";
    }

    out() << ")";
    return false;
}

ReturnType ASTRender::constant(Constant MAYBE_CONST* self, int depth) {
    StringStream ss;
    self->value.print(ss);
    out() << ss.str();
    return false;
}

ASTRender::ExprRet ASTRender::placeholder(Placeholder_t* self, int depth) { return false; }

ReturnType ASTRender::namedexpr(NamedExpr MAYBE_CONST* self, int depth) {
    run(self->target, depth);
    out() << " := ";
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::classdef(ClassDef MAYBE_CONST* self, int depth) {
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

    for (auto MAYBE_CONST& kw: self->keywords) {
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
        Docstring MAYBE_CONST& doc = self->docstring.value();
        out() << indentation << "\"\"\"" << doc.docstring << "\"\"\"";

        maybe_inline_comment(doc.comment, depth, LOC);
        out() << newline;
    }

    bool assign = false;
    k           = 0;
    for (auto MAYBE_CONST& stmt: self->body) {

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

ReturnType ASTRender::functiondef(FunctionDef MAYBE_CONST* self, int depth) {
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

    out() << special::Keyword("def ") << special::Editable(self->name, (Node*)self) << "(";
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
        Docstring MAYBE_CONST& doc = self->docstring.value();

        {
            auto _ = indent();
            out() << indentation << special::Docstring(doc.docstring);
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

ReturnType ASTRender::inlinestmt(Inline MAYBE_CONST* self, int depth) {
    out() << indentation;

    int k = 0;
    for (auto MAYBE_CONST& stmt: self->body) {
        run(stmt, depth);

        if (k + 1 < self->body.size()) {
            out() << "; ";
        }

        k += 1;
    }

    return false;
}

ReturnType ASTRender::forstmt(For MAYBE_CONST* self, int depth) {
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

ReturnType ASTRender::trystmt(Try MAYBE_CONST* self, int depth) {
    out() << special::Keyword("try:");
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    {
        auto _ = indent();
        render_body(self->body, depth + 1);
    }

    for (auto MAYBE_CONST& handler: self->handlers) {
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

ReturnType ASTRender::compare(Compare MAYBE_CONST* self, int depth) {
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

ReturnType ASTRender::binop(BinOp MAYBE_CONST* self, int depth) {
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

    print_op(self->op, false);

    if (rhspred) {
        out() << '(';
    }
    run(self->right, depth);
    if (rhspred) {
        out() << ')';
    }

    return false;
}

ReturnType ASTRender::invalidstmt(InvalidStatement MAYBE_CONST* self, int depth) {
    //
    if (!self->tokens.empty()) {
        Unlex        unlex;
        StringStream ss;
        unlex.format(ss, self->tokens);
        out() << ss.str();
    }
    return false;
}

ReturnType ASTRender::boolop(BoolOp MAYBE_CONST* self, int depth) {

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

ReturnType ASTRender::unaryop(UnaryOp MAYBE_CONST* self, int depth) {
    print_op(self->op);
    out() << " ";
    run(self->operand, depth);

    return false;
}

ReturnType ASTRender::whilestmt(While MAYBE_CONST* self, int depth) {
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

ReturnType ASTRender::returnstmt(Return MAYBE_CONST* self, int depth) {
    out() << special::Keyword("return ");

    if (self->value.has_value()) {
        run(self->value.value(), depth);
    }

    return false;
}

ReturnType ASTRender::deletestmt(Delete MAYBE_CONST* self, int depth) {
    out() << special::Keyword("del ");

    for (int i = 0; i < self->targets.size(); i++) {
        run(self->targets[i], depth);

        if (i < self->targets.size() - 1)
            out() << ", ";
    }

    return false;
}

ReturnType ASTRender::augassign(AugAssign MAYBE_CONST* self, int depth) {
    run(self->target, depth);
    print_op(self->op, true);
    out() << "= ";
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::assign(Assign MAYBE_CONST* self, int depth) {
    run(self->targets[0], depth);
    out() << " = ";
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::annassign(AnnAssign MAYBE_CONST* self, int depth) {
    run(self->target, depth);
    out() << ": ";

    run(self->annotation, depth);
    if (self->value.has_value()) {
        out() << " = ";
        run(self->value.value(), depth);
    }

    return false;
}

ReturnType ASTRender::pass(Pass MAYBE_CONST* self, int depth) {
    out() << special::Keyword("pass");
    return false;
}

ReturnType ASTRender::breakstmt(Break MAYBE_CONST* self, int depth) {
    out() << special::Keyword("break");
    return false;
}

ReturnType ASTRender::continuestmt(Continue MAYBE_CONST* self, int depth) {
    out() << special::Keyword("continue");
    return false;
}

ReturnType ASTRender::exprstmt(Expr MAYBE_CONST* self, int depth) {
    if (self->value != nullptr)
        run(self->value, depth);

    return false;
}

ReturnType ASTRender::global(Global MAYBE_CONST* self, int depth) {
    out() << special::Keyword("global ") << join(", ", self->names);
    return false;
}

ReturnType ASTRender::nonlocal(Nonlocal MAYBE_CONST* self, int depth) {
    out() << special::Keyword("nonlocal ") << join(", ", self->names);
    return false;
}

ReturnType ASTRender::arrow(Arrow MAYBE_CONST* self, int depth) {
    out() << '(' << join<ExprNode*>(", ", self->args) << ") -> ";
    out() << str(self->returns);
    return false;
}

ReturnType ASTRender::dicttype(DictType MAYBE_CONST* self, int depth) {
    out() << "Dict[";
    out() << str(self->key);
    out() << ", ";
    out() << str(self->value) << "]";
    return false;
}

ReturnType ASTRender::settype(SetType MAYBE_CONST* self, int depth) {
    out() << "Set[";
    out() << str(self->value) << "]";
    return false;
}

ReturnType ASTRender::name(Name MAYBE_CONST* self, int depth) {
    out() << self->id;
    return false;
}

ReturnType ASTRender::arraytype(ArrayType MAYBE_CONST* self, int depth) {
    out() << "Array[";
    out() << str(self->value) << "]";
    return false;
}

ReturnType ASTRender::tupletype(TupleType MAYBE_CONST* self, int depth) {
    out() << "Tuple[";
    out() << join<ExprNode*>(", ", self->types) << "]";
    return false;
}

ReturnType ASTRender::builtintype(BuiltinType MAYBE_CONST* self, int depth) {
    out() << self->name;
    return false;
}

ReturnType ASTRender::joinedstr(JoinedStr MAYBE_CONST* self, int depth) {
    out() << "f\"";

    for (auto* val: self->values) {
        if (Constant* cst = cast<Constant>(val)) {
            out() << cst->value.get<String>();
        } else {
            run(val, depth);
        }
    }

    out() << '"';
    return false;
}

ReturnType ASTRender::formattedvalue(FormattedValue MAYBE_CONST* self, int depth) {
    out() << "{";
    run(self->value, depth);
    out() << ":";

    for (auto* val: self->format_spec->values) {
        if (Constant* cst = cast<Constant>(val)) {
            out() << cst->value.get<String>();
        } else {
            out() << "{";
            run(val, depth);
            out() << "}";
        }
    }

    out() << "}";
    return false;
}

ReturnType ASTRender::classtype(ClassType MAYBE_CONST* self, int depth) {
    out() << self->def->name;
    return false;
}

ReturnType ASTRender::exported(Exported MAYBE_CONST* self, int depth) {
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

void ASTRender::print_op(BinaryOperator op, bool aug) {
    // clang-format off
    switch (op) {
    case BinaryOperator::Add:       out() << " +"; break;
    case BinaryOperator::Sub:       out() << " -"; break;
    case BinaryOperator::Mult:      out() << " *"; break;
    case BinaryOperator::MatMult:   out() << " @"; break;
    case BinaryOperator::Div:       out() << " /"; break;
    case BinaryOperator::Mod:       out() << " %"; break;
    case BinaryOperator::Pow:       out() << " **";  break;
    case BinaryOperator::LShift:    out() << " <<";  break;
    case BinaryOperator::RShift:    out() << " >>";  break;
    case BinaryOperator::BitOr:     out() << " |";   break;
    case BinaryOperator::BitXor:    out() << " ^";   break;
    case BinaryOperator::BitAnd:    out() << " &";   break;
    case BinaryOperator::FloorDiv:  out() << " //";  break;
    case BinaryOperator::EltMult:   out() << " .*";  break;
    case BinaryOperator::EltDiv:    out() << " ./";  break;
    case BinaryOperator::None:      out() << " <Binary:None>"; break;
    }
    // clang-format on

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

void ASTRender::keyword(Keyword MAYBE_CONST& self, int depth) {
    out() << self.arg;
    if (self.value != nullptr) {
        out() << " = ";
        run(self.value, depth);
    }
}

void ASTRender::alias(Alias MAYBE_CONST& self, int depth) {
    out() << self.name;
    if (self.asname.has_value()) {
        out() << special::Keyword(" as ") << self.asname.value();
    }
}

ReturnType ASTRender::functiontype(FunctionType MAYBE_CONST* self, int depth) { return false; }

ReturnType ASTRender::expression(Expression MAYBE_CONST* self, int depth) { return false; }

ReturnType ASTRender::interactive(Interactive MAYBE_CONST* self, int depth) { return false; }

void ASTRender::withitem(WithItem MAYBE_CONST& self, int depth) {
    run(self.context_expr, depth);
    if (self.optional_vars.has_value()) {
        out() << special::Keyword(" as ");
        run(self.optional_vars.value(), depth);
    }
}

ReturnType ASTRender::comment(Comment MAYBE_CONST* n, int depth) {
    out() << "#" << n->comment;
    return false;
}

void ASTRender::arguments(Arguments MAYBE_CONST& self, int depth) {
    int i = 0;

    for (auto& arg: self.args) {
        auto config = special::Editable(arg.arg);
        config.backspace = [&]() {
            String string = str(arg.arg);
            arg.arg = StringRef(string.substr(0, string.size() - 1));
            _redraw = true;
        };
        config.input = [&](unsigned int c) {
            String string = str(arg.arg);
            string.push_back(c);
            arg.arg = StringRef(string);
            _redraw = true;
        };
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
    for (auto MAYBE_CONST& kw: self.kwonlyargs) {
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

}  // namespace lython
