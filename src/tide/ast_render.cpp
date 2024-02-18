#include "tide/ast_render.h"

#include "ast/magic.h"
#include "ast/nodes.h"
#include "ast/values/native.h"
#include "ast/values/object.h"
#include "ast/visitor.h"
#include "dependencies/fmt.h"
#include "lexer/unlex.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/allocator.h"
#include "utilities/strings.h"
#include "ast/ops.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>


namespace lython {


ASTRenderContext& ASTRenderContext::operator<< (special::Indent const& str) {
    String idt(4 * _indent, ' ');
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, idt.c_str());
    pos += ImVec2(size.x, 0);
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<< (special::BeforeComment const& str) {
    String idt = "   ";
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, idt.c_str());

    float maybe_x = pos.x + size.x;
    maxcol = std::max(maybe_x, maxcol);

    pos.x = maxcol;

    _comment = true;
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<< (special::Newline const& str) {
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, " ");

    if (!_comment) {
        maxcol = std::max(pos.x, maxcol);
    }
    _comment = false;

    pos.x = start.x;
    pos.y += (size.y + style->extra_line_space);
    return (*this);
}

void ASTRenderContext::inline_string(const char* str, ImColor color) 
{
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, str);

    ImRect bb(pos, pos + size);

    unsigned int id = _id;
    ImGui::ItemAdd(bb, id);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

    //ImGui::InputText();


    ImGuiContext* g = ImGui::GetCurrentContext();
    ImGuiInputTextState* state = ImGui::GetInputTextState(id);
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImGuiIO& io = ImGui::GetIO();
    int flags;

    if (hovered) {
        g->MouseCursor = ImGuiMouseCursor_TextInput;
    }

    if (g->LastItemData.InFlags & ImGuiItemFlags_ReadOnly)
        flags |= ImGuiInputTextFlags_ReadOnly;

    const bool is_readonly = (flags & ImGuiInputTextFlags_ReadOnly) != 0;
    const bool is_password = (flags & ImGuiInputTextFlags_Password) != 0;
    const bool is_undoable = (flags & ImGuiInputTextFlags_NoUndoRedo) == 0;
    const bool is_resizable = (flags & ImGuiInputTextFlags_CallbackResize) != 0;

    if (is_resizable) {
        //IM_ASSERT(callback != NULL); // Must provide a callback if you set the ImGuiInputTextFlags_CallbackResize flag!
    }

    const bool input_requested_by_nav = 
         (g->ActiveId != id) && 
        ((g->NavActivateId == id) && 
        ((g->NavActivateFlags & ImGuiActivateFlags_PreferInput) || (g->NavInputSource == ImGuiInputSource_Keyboard)));

    const bool user_clicked = hovered && io.MouseClicked[0];
    const bool user_scroll_finish = false;
    const bool user_scroll_active = false;
    bool clear_active_id = false;
    bool select_all = false;

    float scroll_y =  FLT_MAX;

    const bool init_reload_from_user_buf = (state != NULL && state->ReloadUserBuf);
    const bool init_changed_specs = (state != NULL && state->Stb.single_line != true); // state != NULL means its our state.
    const bool init_make_active = (user_clicked || user_scroll_finish || input_requested_by_nav);
    const bool init_state = (init_make_active || user_scroll_active);

    if (g->ActiveId != id && init_make_active)
    {
        // IM_ASSERT(state && state->ID == id);
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
    }

    if (g->ActiveId == id)
    {
    }

    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    drawlist->AddText(
        style->font, 
        style->font_size + int(hovered),
        pos, 
        color, 
        str
    );

    _id += 1;
    pos += ImVec2(size.x, 0);
}

ASTRenderContext& ASTRenderContext::operator<< (special::Keyword const& keyword) {
    const char* key =  keyword.name.c_str();
    inline_string(key, style->keyword);
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<< (special::Docstring const& keyword) {
    String docstring = String("\"\"\"") + keyword.name + String("\"\"\"");

    const char* key =  docstring.c_str(); 
    inline_string(key, style->docstring);
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<< (special::Type const& keyword) {
    const char* key =  keyword.name.c_str();
    inline_string(key, style->type);
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<< (const char* str) {
    ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    inline_string(str, ImColor(color.x, color.y, color.z));
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<< (String const& str) {
    return (*this) << str.c_str();
}


void print_op(ASTRenderContext& out, UnaryOperator op);
void print_op(ASTRenderContext& out, CmpOperator op);
void print_op(ASTRenderContext& out, BinaryOperator op, bool aug);
void print_op(ASTRenderContext& out, BoolOperator op);

special::Newline       newline;
special::Indent        indentation;
special::BeforeComment comment_space;

void ASTRender::maybe_inline_comment(
    Comment* com, int depth, ASTRenderContext& out, int level, CodeLocation const& loc) {
    if (com != nullptr) {
        // lython::log(lython::LogLevel::Info, loc, "printing inline comment {}", com->comment);
        out << comment_space;
        {
            auto _ = out.comment();
            exec(com, depth, out, level);
        }
    }
}

ReturnType ASTRender::render_body(
    Array<StmtNode*> const& body, int depth, ASTRenderContext& out, int level, bool print_last) {

    int k = 0;
    for (auto const& stmt: body) {
        k += 1;

        out << indentation;
        bool printed_new_line = exec(stmt, depth, out, level);

        if (stmt->is_one_line()) {
            maybe_inline_comment(stmt->comment, depth, out, level, LOC);
        }

        // out << newline;
        if (!printed_new_line) {
            if (k < body.size() || print_last) {
                out << newline;
            }
        }
    }

    return true;
}

ReturnType
ASTRender::excepthandler(ExceptHandler const& self, int depth, ASTRenderContext& out, int level) {
    out << newline << indentation << special::Keyword("except ");

    if (self.type.has_value()) {
        exec(self.type.value(), depth, out, level);
    }

    if (self.name.has_value()) {
        out << special::Keyword(" as ");
        out << self.name.value();
    }

    out << ":";
    maybe_inline_comment(self.comment, depth, out, level, LOC);
    out << newline;

    {
        auto _ = out.indent();
        return render_body(self.body, depth, out, level + 1);
    }
    
}

ReturnType
ASTRender::matchcase(MatchCase const& self, int depth, ASTRenderContext& out, int level) {
    out << indentation << special::Keyword("case ");
    exec(self.pattern, depth, out, level);

    if (self.guard.has_value()) {
        out << special::Keyword(" if ");
        exec(self.guard.value(), depth, out, level);
    }

    out << ":";
    maybe_inline_comment(self.comment, depth, out, level, LOC);
    out << newline;
    
    {
        auto _ = out.indent();
        return render_body(self.body, depth, out, level + 1);
    }
}

void ASTRender::arg(Arg const& self, int depth, ASTRenderContext& out, int level) {
    out << self.arg;

    if (self.annotation.has_value()) {
        out << ": ";
        exec(self.annotation.value(), depth, out, level);
    }
}

ReturnType
ASTRender::attribute(Attribute const* self, int depth, ASTRenderContext& out, int level) {
    exec(self->value, depth, out, level);
    out << ".";
    out << self->attr;
    return false;
}

ReturnType
ASTRender::subscript(Subscript const* self, int depth, ASTRenderContext& out, int level) {
    exec(self->value, depth, out, level);
    out << "[";
    exec(self->slice, depth, out, level);
    out << "]";
    return false;
}

ReturnType ASTRender::starred(Starred const* self, int depth, ASTRenderContext& out, int level) {
    out << "*";
    exec(self->value, depth, out, level);
    return false;
}

ReturnType ASTRender::module(Module const* self, int depth, ASTRenderContext& out, int level) {
    return render_body(self->body, depth, out, level);
}

ReturnType ASTRender::raise(Raise const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("raise ");
    if (self->exc.has_value()) {
        exec(self->exc.value(), depth, out, level);
    }

    if (self->cause.has_value()) {
        out << special::Keyword(" from ");
        exec(self->cause.value(), depth, out, level);
    }
    return false;
}

ReturnType ASTRender::assertstmt(Assert const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("assert ");
    exec(self->test, depth, out, level);

    if (self->msg.has_value()) {
        out << ", ";
        exec(self->msg.value(), depth, out, level);
    }
    return false;
}

ReturnType ASTRender::with(With const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("with ");

    int i = 0;
    for (auto const& item: self->items) {
        exec(item.context_expr, depth, out, level);

        if (item.optional_vars.has_value()) {
            out << special::Keyword(" as ");
            exec(item.optional_vars.value(), depth, out, level);
        }

        if (i + 1 < self->items.size()) {
            out << ", ";
        }
        i += 1;
    }
    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << newline;

    {
        auto _ = out.indent();
        render_body(self->body, depth, out, level + 1);
    }
    return false;
}

ReturnType ASTRender::import(Import const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("import ");

    int i = 0;
    for (auto const& alias: self->names) {
        out << alias.name;

        if (alias.asname.has_value()) {
            out << special::Keyword(" as ");
            out << alias.asname.value();
        }

        if (i + 1 < self->names.size()) {
            out << ", ";
        }
        i += 1;
    }
    return false;
}

ReturnType
ASTRender::importfrom(ImportFrom const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("from ");
    if (self->module.has_value()) {
        out << self->module.value();
    }
    out << special::Keyword(" import ");

    int i = 0;
    for (auto const& alias: self->names) {
        out << alias.name;

        if (alias.asname.has_value()) {
            out << special::Keyword(" as ");
            out << alias.asname.value();
        }

        if (i + 1 < self->names.size()) {
            out << ", ";
        }
        i += 1;
    }
    return false;
}

ReturnType ASTRender::slice(Slice const* self, int depth, ASTRenderContext& out, int level) {
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

ReturnType
ASTRender::tupleexpr(TupleExpr const* self, int depth, ASTRenderContext& out, int level) {
    if (level == -1) {
        out << join<ExprNode*>(", ", self->elts);
    } else {
        out << "(" << join<ExprNode*>(", ", self->elts) << ")";
    }
    return false;
}

ReturnType ASTRender::listexpr(ListExpr const* self, int depth, ASTRenderContext& out, int level) {
    out << "[" << join<ExprNode*>(", ", self->elts) << "]";
    return false;
}

ReturnType ASTRender::setexpr(SetExpr const* self, int depth, ASTRenderContext& out, int level) {
    out << "{" << join<ExprNode*>(", ", self->elts) << "}";
    return false;
}

ReturnType ASTRender::dictexpr(DictExpr const* self, int depth, ASTRenderContext& out, int level) {
    Array<String> strs;
    strs.reserve(self->keys.size());

    for (int i = 0; i < self->keys.size(); i++) {
        strs.push_back(fmtstr("{}: {}", str(self->keys[i]), str(self->values[i])));
    }

    out << "{" << join(", ", strs) << "}";
    return false;
}

ReturnType
ASTRender::matchvalue(MatchValue const* self, int depth, ASTRenderContext& out, int level) {
    exec(self->value, depth, out, level);
    return false;
}

ReturnType
ASTRender::matchsingleton(MatchSingleton const* self, int depth, ASTRenderContext& out, int level) {
    StringStream ss;
    self->value.print(ss);
    out << ss.str();
    return false;
}

ReturnType
ASTRender::matchsequence(MatchSequence const* self, int depth, ASTRenderContext& out, int level) {
    auto result = join(", ", self->patterns);
    out << "[" << result << "]";
    return false;
}

ReturnType
ASTRender::matchmapping(MatchMapping const* self, int depth, ASTRenderContext& out, int level) {
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

ReturnType
ASTRender::matchclass(MatchClass const* self, int depth, ASTRenderContext& out, int level) {
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

ReturnType
ASTRender::matchstar(MatchStar const* self, int depth, ASTRenderContext& out, int level) {
    out << "*";

    if (self->name.has_value()) {
        out << self->name.value();
    }
    return false;
}

ReturnType ASTRender::matchas(MatchAs const* self, int depth, ASTRenderContext& out, int level) {
    if (self->pattern.has_value()) {
        exec(self->pattern.value(), depth, out, level);
    }

    if (self->name.has_value()) {
        out << special::Keyword(" as ") << self->name.value();
    }
    return false;
}

ReturnType ASTRender::matchor(MatchOr const* self, int depth, ASTRenderContext& out, int level) {
    out << join(" | ", self->patterns);
    return false;
}

ReturnType ASTRender::ifstmt(If const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("if ");
    exec(self->test, depth, out, level);
    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << newline;

    {
        auto _ = out.indent();
        render_body(self->body, depth, out, level + 1);
    }

    for (int i = 0; i < self->tests.size(); i++) {
        auto& eliftest = self->tests[i];
        auto& elifbody = self->bodies[i];

        out << newline << indentation << special::Keyword("elif ");

        exec(eliftest, depth, out, level);
        out << ":";
        maybe_inline_comment(self->tests_comment[i], depth, out, level, LOC);
        out << newline;
        {
            auto _ = out.indent();
            render_body(elifbody, depth, out, level + 1);
        }
    }

    if (!self->orelse.empty()) {
        out << newline << indentation << special::Keyword("else:");
        maybe_inline_comment(self->else_comment, depth, out, level, LOC);
        out << newline;

        {   
            auto _ = out.indent();
            render_body(self->orelse, depth, out, level + 1);
        }
    }
    return false;
}

ReturnType ASTRender::match(Match const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("match ");
    exec(self->subject, depth, out, level);
    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << newline;

    int i = 0;
    for (auto const& case_: self->cases) {
        matchcase(case_, depth, out, level + 1);

        if (i + 1 < self->cases.size()) {
            out << newline;
        }

        i += 1;
    }
    return false;
}

ReturnType ASTRender::lambda(Lambda const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("lambda ");
    arguments(self->args, depth, out, 0);
    out << ": ";
    exec(self->body, depth, out, level);
    return false;
}

ReturnType ASTRender::ifexp(IfExp const* self, int depth, ASTRenderContext& out, int level) {
    exec(self->body, depth, out, level);
    out << special::Keyword(" if ");
    exec(self->test, depth, out, level);
    out << special::Keyword(" else ");
    exec(self->orelse, depth, out, level);
    return false;
}

void ASTRender::comprehension(Comprehension const& self, int depth, ASTRenderContext& out, int level) 
{
    out << special::Keyword(" for ");
    exec(self.target, depth, out, level);
    out << special::Keyword(" in ");
    exec(self.iter, depth, out, level);

    for (auto* expr: self.ifs) {
        out << special::Keyword(" if ");
        exec(expr, depth, out, level);
    }
}

void ASTRender::comprehensions(Array<Comprehension> const& self, int depth, ASTRenderContext& out, int level) 
{
    for(Comprehension const& comp: self) {
        comprehension(comp, depth, out, level);
    }
}

ReturnType ASTRender::listcomp(ListComp const* self, int depth, ASTRenderContext& out, int level) {
    out << "[";
    exec(self->elt, depth, out, level);

    comprehensions(self->generators, depth, out, level);

    out << "]";
    return false;
}

ReturnType ASTRender::setcomp(SetComp const* self, int depth, ASTRenderContext& out, int level) {
    out << "{";
    exec(self->elt, depth, out, level);

    comprehensions(self->generators, depth, out, level);

    out << "}";
    return false;
}

ReturnType
ASTRender::generateexpr(GeneratorExp const* self, int depth, ASTRenderContext& out, int level) {
    out << "(";
    exec(self->elt, depth, out, level);

    comprehensions(self->generators, depth, out, level);

    out << ")";
    return false;
}

ReturnType ASTRender::dictcomp(DictComp const* self, int depth, ASTRenderContext& out, int level) {
    out << "{";
    exec(self->key, depth, out, level);
    out << ": ";
    exec(self->value, depth, out, level);

    comprehensions(self->generators, depth, out, level);
    out << "}";
    return false;
}

ReturnType ASTRender::await(Await const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("await ");
    exec(self->value, depth, out, level);
    return false;
}

ReturnType ASTRender::yield(Yield const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("yield");
    if (self->value.has_value()) {
        out << " ";
        exec(self->value.value(), depth, out, level);
    }
    return false;
}

ReturnType
ASTRender::yieldfrom(YieldFrom const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("yield from ");
    exec(self->value, depth, out, level);
    return false;
}

ReturnType ASTRender::call(Call const* self, int depth, ASTRenderContext& out, int level) {

    // StringStream fname = fmt(self->func, depth, level);
    // out << fname.str() << "(";

    exec(self->func, depth, out, level);
    out << "(";

    for (int i = 0; i < self->args.size(); i++) {
        exec(self->args[i], depth, out, level);

        if (i < self->args.size() - 1 || !self->keywords.empty())
            out << ", ";
    }

    for (int i = 0; i < self->keywords.size(); i++) {
        auto const& key = self->keywords[i];

        out << self->keywords[i].arg;
        out << "=";

        exec(key.value, depth, out, level);

        if (i < self->keywords.size() - 1)
            out << ", ";
    }

    out << ")";
    return false;
}

ReturnType ASTRender::constant(Constant const* self, int depth, ASTRenderContext& out, int level) {
    StringStream ss;
    self->value.print(ss);
    out << ss.str();
    return false;
}

ASTRender::ExprRet
ASTRender::placeholder(Placeholder_t* self, int depth, ASTRenderContext& out, int level) {
    return false;
}

ReturnType
ASTRender::namedexpr(NamedExpr const* self, int depth, ASTRenderContext& out, int level) {
    exec(self->target, depth, out, level);
    out << " := ";
    exec(self->value, depth, out, level);
    return false;
}

ReturnType ASTRender::classdef(ClassDef const* self, int depth, ASTRenderContext& out, int level) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out << indentation;
        }

        out << "@";
        exec(decorator.expr, depth, out, level);
        maybe_inline_comment(decorator.comment, depth, out, level, LOC);
        out << newline;
        k += 1;
    }

    if (!self->decorator_list.empty()) {
        out << indentation;
    }

    out << special::Keyword("class ") << self->name;
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
    out << newline;

    if (self->docstring.has_value()) {
        Docstring const& doc = self->docstring.value();
        out << indentation << "\"\"\"" << doc.docstring << "\"\"\"";

        maybe_inline_comment(doc.comment, depth, out, level, LOC);
        out << newline;
    }

    bool assign = false;
    k           = 0;
    for (auto const& stmt: self->body) {

        assign = stmt->kind == NodeKind::Assign || stmt->kind == NodeKind::AnnAssign ||
                 stmt->kind == NodeKind::Pass;

        // print an extra line before if not an attribute
        if (k > 0 && !assign) {
            out << newline;
        }

        out << indentation;  // indent(level + 1);
        bool printed_new_line = exec(stmt, depth, out, level + 1);

        if (stmt->is_one_line()) {
            maybe_inline_comment(stmt->comment, depth, out, level, LOC);
        }
        if (!printed_new_line) {
            if (k + 1 < self->body.size()) {
                out << newline;
            }
        }
        k += 1;
    }
    return false;
}

ReturnType
ASTRender::functiondef(FunctionDef const* self, int depth, ASTRenderContext& out, int level) {
    int k = 0;
    for (auto decorator: self->decorator_list) {
        if (k > 0) {
            out << indentation;
        }

        out << "@";
        exec(decorator.expr, depth, out, level);
        maybe_inline_comment(decorator.comment, depth, out, level, LOC);
        out << newline;
        k += 1;
    }

    if (!self->decorator_list.empty()) {
        out << indentation;
    }

    out << special::Keyword("def ") << self->name << "(";
    arguments(self->args, depth, out, level);
    out << ")";

    if (self->returns.has_value()) {
        out << " -> ";
        {
            auto _ = out.type();
            exec(self->returns.value(), depth, out, level);
        }
    }

    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << newline;

    if (self->docstring.has_value()) {
        Docstring const& doc = self->docstring.value();
        
        {
            auto _ = out.indent();
            out << indentation << special::Docstring(doc.docstring);
        }

        maybe_inline_comment(doc.comment, depth, out, level, LOC);
        out << newline;
    }

    { 
        auto _ = out.indent();
        render_body(self->body, depth, out, level + 1, true);
    }

    out << newline;
    return false;
}

ReturnType ASTRender::inlinestmt(Inline const* self, int depth, ASTRenderContext& out, int level) {
    out << indentation;

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

ReturnType ASTRender::forstmt(For const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("for ");
    exec(self->target, depth, out, -1);
    out << special::Keyword(" in ");
    exec(self->iter, depth, out, -1);
    out << ":";

    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << newline;

    {
        auto _ = out.indent();
        render_body(self->body, depth, out, level + 1);
    }

    if (!self->orelse.empty()) {
        out << newline;
        out << indentation << special::Keyword("else:");
        maybe_inline_comment(self->else_comment, depth, out, level, LOC);
        out << newline;

        {
            auto _ = out.indent();
            render_body(self->orelse, depth, out, level + 1);
        }
    }

    return false;
}

ReturnType ASTRender::trystmt(Try const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("try:");
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << newline;

    {
        auto _ = out.indent();
        render_body(self->body, depth, out, level + 1);
    }

    for (auto const& handler: self->handlers) {
        excepthandler(handler, depth, out, level);
    }

    if (!self->orelse.empty()) {
        out << newline << indentation << special::Keyword("else:");
        maybe_inline_comment(self->else_comment, depth, out, level, LOC);
        out << newline;

        {
            auto _ = out.indent();
            render_body(self->orelse, depth, out, level + 1);
        }
    }

    if (!self->finalbody.empty()) {
        out << newline << indentation << special::Keyword("finally:");
        maybe_inline_comment(self->finally_comment, depth, out, level, LOC);
        out << newline;

        {
            auto _ = out.indent();
            render_body(self->finalbody, depth, out, level + 1);
        }
    }

    return false;
}

ReturnType ASTRender::compare(Compare const* self, int depth, ASTRenderContext& out, int level) {
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

ReturnType ASTRender::binop(BinOp const* self, int depth, ASTRenderContext& out, int level) {
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
ASTRender::invalidstmt(InvalidStatement const* self, int depth, ASTRenderContext& out, int level) {
    //
    if (!self->tokens.empty()) {
        Unlex        unlex;
        StringStream ss;
        unlex.format(ss, self->tokens);
        out << ss.str();
    }
    return false;
}

ReturnType ASTRender::boolop(BoolOp const* self, int depth, ASTRenderContext& out, int level) {

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

ReturnType ASTRender::unaryop(UnaryOp const* self, int depth, ASTRenderContext& out, int level) {
    print_op(out, self->op);
    out << " ";
    exec(self->operand, depth, out, level);

    return false;
}

ReturnType ASTRender::whilestmt(While const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("while ");
    exec(self->test, depth, out, level);
    out << ":";
    maybe_inline_comment(self->comment, depth, out, level, LOC);
    out << newline;
    {
        auto _ = out.indent();
        render_body(self->body, depth, out, level + 1);
    }
    
    if (!self->orelse.empty()) {
        out << newline << indentation << special::Keyword("else:");
        maybe_inline_comment(self->else_comment, depth, out, level, LOC);
        out << newline;

        {
            auto _ = out.indent();
            render_body(self->orelse, depth, out, level + 1);
        }
    }

    return false;
}

ReturnType ASTRender::returnstmt(Return const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("return ");

    if (self->value.has_value()) {
        exec(self->value.value(), depth, out, -1);
    }

    return false;
}

ReturnType ASTRender::deletestmt(Delete const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("del ");

    for (int i = 0; i < self->targets.size(); i++) {
        exec(self->targets[i], depth, out, level);

        if (i < self->targets.size() - 1)
            out << ", ";
    }

    return false;
}

ReturnType
ASTRender::augassign(AugAssign const* self, int depth, ASTRenderContext& out, int level) {
    exec(self->target, depth, out, -1);
    print_op(out, self->op, true);
    out << "= ";
    exec(self->value, depth, out, -1);
    return false;
}

ReturnType ASTRender::assign(Assign const* self, int depth, ASTRenderContext& out, int level) {
    exec(self->targets[0], depth, out, -1);
    out << " = ";
    exec(self->value, depth, out, -1);
    return false;
}

ReturnType
ASTRender::annassign(AnnAssign const* self, int depth, ASTRenderContext& out, int level) {
    exec(self->target, depth, out, level);
    out << ": ";

    exec(self->annotation, depth, out, level);
    if (self->value.has_value()) {
        out << " = ";
        exec(self->value.value(), depth, out, level);
    }

    return false;
}

ReturnType ASTRender::pass(Pass const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("pass");
    return false;
}

ReturnType ASTRender::breakstmt(Break const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("break");
    return false;
}

ReturnType
ASTRender::continuestmt(Continue const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("continue");
    return false;
}

ReturnType ASTRender::exprstmt(Expr const* self, int depth, ASTRenderContext& out, int level) {
    if (self->value != nullptr)
        exec(self->value, depth, out, -1);

    return false;
}

ReturnType ASTRender::global(Global const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("global ") << join(", ", self->names);
    return false;
}

ReturnType ASTRender::nonlocal(Nonlocal const* self, int depth, ASTRenderContext& out, int level) {
    out << special::Keyword("nonlocal ") << join(", ", self->names);
    return false;
}

ReturnType ASTRender::arrow(Arrow const* self, int depth, ASTRenderContext& out, int level) {
    out << '(' << join<ExprNode*>(", ", self->args) << ") -> ";
    out << str(self->returns);
    return false;
}

ReturnType ASTRender::dicttype(DictType const* self, int depth, ASTRenderContext& out, int level) {
    out << "Dict[";
    out << str(self->key);
    out << ", ";
    out << str(self->value) << "]";
    return false;
}

ReturnType ASTRender::settype(SetType const* self, int depth, ASTRenderContext& out, int level) {
    out << "Set[";
    out << str(self->value) << "]";
    return false;
}

ReturnType ASTRender::name(Name const* self, int depth, ASTRenderContext& out, int level) {
    out << self->id;
    return false;
}

ReturnType
ASTRender::arraytype(ArrayType const* self, int depth, ASTRenderContext& out, int level) {
    out << "Array[";
    out << str(self->value) << "]";
    return false;
}

ReturnType
ASTRender::tupletype(TupleType const* self, int depth, ASTRenderContext& out, int level) {
    out << "Tuple[";
    out << join<ExprNode*>(", ", self->types) << "]";
    return false;
}

ReturnType
ASTRender::builtintype(BuiltinType const* self, int depth, ASTRenderContext& out, int level) {
    out << self->name;
    return false;
}

ReturnType
ASTRender::joinedstr(JoinedStr const* self, int depth, ASTRenderContext& out, int level) {
    out << "f\"";

    for (auto* val: self->values) {
        if (Constant* cst = cast<Constant>(val)) {
            out << cst->value.get<String>();
        } else {
            exec(val, depth, out, level);
        }
    }

    out << '"';
    return false;
}

ReturnType ASTRender::formattedvalue(FormattedValue const* self,
                                     int                   depth,
                                     ASTRenderContext&     out,
                                     int                   indent) {
    out << "{";
    exec(self->value, depth, out, indent);
    out << ":";

    for (auto* val: self->format_spec->values) {
        if (Constant* cst = cast<Constant>(val)) {
            out << cst->value.get<String>();
        } else {
            out << "{";
            exec(val, depth, out, indent);
            out << "}";
        }
    }

    out << "}";
    return false;
}

ReturnType
ASTRender::classtype(ClassType const* self, int depth, ASTRenderContext& out, int level) {
    out << self->def->name;
    return false;
}

ReturnType ASTRender::exported(Exported const* self, int depth, ASTRenderContext& out, int level) {
    // exec(self->node, depth, out, level);
    return false;
}

// Helper
// ==================================================

void print_op(ASTRenderContext& out, BoolOperator op) {
    // clang-format off
    switch (op) {
    case BoolOperator::And:   out << special::Keyword(" and "); return;
    case BoolOperator::Or:    out << special::Keyword(" or ") ; return;
    case BoolOperator::None:  out << " <Bool:None> " ; return;
    }
    // clang-format on
}

void print_op(ASTRenderContext& out, BinaryOperator op, bool aug) {
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

void print_op(ASTRenderContext& out, CmpOperator op) {
    // clang-format off
    switch (op) {
    case CmpOperator::None:     out << " <Cmp:None> "; return;
    case CmpOperator::Eq:       out << " == ";  return;
    case CmpOperator::NotEq:    out << " != ";  return;
    case CmpOperator::Lt:       out << " < ";   return;
    case CmpOperator::LtE:      out << " <= ";  return;
    case CmpOperator::Gt:       out << " > ";   return;
    case CmpOperator::GtE:      out << " >= ";  return;
    case CmpOperator::Is:       out << special::Keyword(" is ");  return;
    case CmpOperator::IsNot:    out << special::Keyword(" is not ");  return;
    case CmpOperator::In:       out << special::Keyword(" in ");      return;
    case CmpOperator::NotIn:    out << special::Keyword(" not in ");  return;
    }
    // clang-format on
}

void print_op(ASTRenderContext& out, UnaryOperator op) {
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

void ASTRender::keyword(Keyword const& self, int depth, ASTRenderContext& out, int level) {
    out << self.arg;
    if (self.value != nullptr) {
        out << " = ";
        exec(self.value, depth, out, level);
    }
}

void ASTRender::alias(Alias const& self, int depth, ASTRenderContext& out, int level) {
    out << self.name;
    if (self.asname.has_value()) {
        out << special::Keyword(" as ") << self.asname.value();
    }
}

ReturnType
ASTRender::functiontype(FunctionType const* self, int depth, ASTRenderContext& out, int indent) {
    return false;
}

ReturnType
ASTRender::expression(Expression const* self, int depth, ASTRenderContext& out, int level) {
    return false;
}

ReturnType
ASTRender::interactive(Interactive const* self, int depth, ASTRenderContext& out, int level) {
    return false;
}

void ASTRender::withitem(WithItem const& self, int depth, ASTRenderContext& out, int level) {
    exec(self.context_expr, depth, out, level);
    if (self.optional_vars.has_value()) {
        out << special::Keyword(" as ");
        exec(self.optional_vars.value(), depth, out, level);
    }
}

ReturnType ASTRender::comment(Comment const* n, int depth, ASTRenderContext& out, int level) {
    out << "#" << n->comment;
    return false;
}

void ASTRender::arguments(Arguments const& self, int depth, ASTRenderContext& out, int level) {
    int i = 0;

    for (auto& arg: self.args) {
        out << arg.arg;

        if (arg.annotation.has_value()) {
            out << ": ";

            {
                auto _ = out.type();
                exec(arg.annotation.value(), depth, out, level);
            }    
        }

        auto default_offset = self.args.size() - 1 - i;
        if (self.defaults.size() > 0 && default_offset < self.defaults.size()) {
            if (arg.annotation.has_value()) {
                out << " = ";
            } else {
                out << "=";
            }
            exec(self.defaults[default_offset], depth, out, -1);
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
    for (auto const& kw: self.kwonlyargs) {
        out << kw.arg;

        if (kw.annotation.has_value()) {
            out << ": ";
            exec(kw.annotation.value(), depth, out, level);
        }

        auto default_offset = self.kwonlyargs.size() - 1 - i;
        if (self.kw_defaults.size() > 0 && default_offset < self.kw_defaults.size()) {
            if (kw.annotation.has_value()) {
                out << " = ";
            } else {
                out << "=";
            }
            exec(self.kw_defaults[default_offset], depth, out, -1);
        }

        if (i + 1 < self.kwonlyargs.size()) {
            out << ", ";
        }
        i += 1;
    }

    if (self.kwarg.has_value()) {
        if (!self.kwonlyargs.empty()) {
            out << ", ";
        }
        out << "**" << self.kwarg.value().arg;
    }
}

}  // namespace lython
