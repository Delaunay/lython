#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

#include "tide/ast_input.h"
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


bool ASTInputText(
    const char* label, 
    const char* hint, 
    char* buf, int buf_size, 
    const ImVec2& size_arg, 
    ImGuiInputTextFlags flags,
    ImGuiInputTextCallback callback, 
    void* callback_user_data
);



#define LOG(X) std::cout << X << std::endl;

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
#if 0
    char buffer[64];
    strcpy(buffer, str);
    ImVec2 size;
    ASTInputText(
        "label", // const char* label, 
        "hint", // const char* hint, 
        (char*)buffer, //char* buf, 
        64, //int buf_size, 
        size, //const ImVec2& size_arg, 
        0, //ImGuiInputTextFlags flags, 
        nullptr, //ImGuiInputTextCallback callback, 
        nullptr //void* callback_user_data
    );

    _id += 1;
    pos += ImVec2(size.x, 0);
    return;
#else
    ImVec2 size = style->font->CalcTextSizeA(
        style->font_size, FLT_MAX, 0.0f, str);
    ImRect bb(pos, pos + size);

    unsigned int id = _id;
    ImGui::ItemAdd(bb, id);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

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


    const char* display_str = str;
    float size_x = size.x;

    static bool prev = user_clicked;

    if (prev != user_clicked) {
        LOG("make active: " << user_clicked);
        prev = user_clicked;
    }

    if (InputState::state().active_id != id && init_make_active)
    {
        // IM_ASSERT(state && state->ID == id);
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
        InputState::state().set(id, str);
    }

    if (InputState::state().active_id == id)
    {
        if (io.InputQueueCharacters.Size > 0) {
            for (int n = 0; n < io.InputQueueCharacters.Size; n++)
            {
                unsigned int c = (unsigned int)io.InputQueueCharacters[n];
                InputState::state().add_character((int)c);
            }
            io.InputQueueCharacters.resize(0);
        }

        display_str = InputState::state().buffer.data();
        float new_size = (size_x / InputState::state().buffer.size()) * InputState::state().buffer.capacity();
        size_x = new_size;
    }

    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    drawlist->AddText(
        style->font, 
        style->font_size + int(hovered),
        pos, 
        color, 
        display_str
    );

    _id += 1;
    pos += ImVec2(size_x, 0);
#endif
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


special::Newline       newline;
special::Indent        indentation;
special::BeforeComment comment_space;

ASTRender& ASTRender::operator<< (const char* name) {
    Drawing& drawing = drawings.emplace_back();
    drawing.string = name;
    drawing.rectangle.Min = cursor;
    //drawing.color = color;
    drawing.style = style;

    ImVec2 size = style->font->CalcTextSizeA(
        style->font_size, FLT_MAX, 0.0f, drawing.string.c_str()
    );

    drawing.rectangle = ImRect(cursor, cursor + size);
    cursor += size;

    return *this;
}
ASTRender& ASTRender::operator<< (String const& name) {
    return *this << name.c_str();
}
ASTRender& ASTRender::operator<< (StringRef const& name) {
    return *this << String(name).c_str();
}
ASTRender& ASTRender::operator<< (special::Indent const& name) {
    String idt(4 * _indent, ' ');
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, idt.c_str());
    cursor += ImVec2(size.x, 0);
    return *this;
}
ASTRender& ASTRender::operator<< (special::Keyword const& keyword) {
    Drawing& drawing = drawings.emplace_back();
    drawing.string = keyword.name;
    return *this;
}
ASTRender& ASTRender::operator<< (special::Newline const& name) {
    ImVec2 size = style->font->CalcTextSizeA(
        style->font_size, FLT_MAX, 0.0f, " ");

    if (!_comment) {
        maxcol = std::max(cursor.x, maxcol);
    }
    _comment = false;

    cursor.x = start.x;
    cursor.y += (size.y + style->extra_line_space);
    return *this;
}
ASTRender& ASTRender::operator<< (special::Docstring const& keyword) {
    Drawing& drawing = drawings.emplace_back();
    drawing.string = String("\"\"\"") + keyword.name + String("\"\"\"");
    return *this;
}
ASTRender& ASTRender::operator<< (special::BeforeComment const& name) {
    String idt = "   ";
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, idt.c_str());

    float maybe_x = cursor.x + size.x;
    maxcol = std::max(maybe_x, maxcol);

    cursor.x = maxcol;

    _comment = true;
    return *this;
}

void Drawing::draw() {
    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    drawlist->AddText(
        style->font, 
        style->font_size,
        rectangle.GetTL(), 
        color, 
        string.c_str()
    );
}

void ASTRender::maybe_inline_comment(
    Comment* com, int depth, CodeLocation const& loc) {
    if (com != nullptr) {
        // lython::log(lython::LogLevel::Info, loc, "printing inline comment {}", com->comment);
        out() << comment_space;
        {
            auto _ = comment();
            run(com, depth);
        }
    }
}

ReturnType ASTRender::render_body(
    Array<StmtNode*> const& body, int depth, bool print_last) {

    int k = 0;
    for (auto const& stmt: body) {
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

    return true;
}

ReturnType
ASTRender::excepthandler(ExceptHandler const& self, int depth) {
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

ReturnType
ASTRender::matchcase(MatchCase const& self, int depth) {
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

void ASTRender::arg(Arg const& self, int depth) {
    out() << self.arg;

    if (self.annotation.has_value()) {
        out() << ": ";
        run(self.annotation.value(), depth);
    }
}

ReturnType
ASTRender::attribute(Attribute const* self, int depth) {
    run(self->value, depth);
    out() << ".";
    out() << self->attr;
    return false;
}

ReturnType
ASTRender::subscript(Subscript const* self, int depth) {
    run(self->value, depth);
    out() << "[";
    run(self->slice, depth);
    out() << "]";
    return false;
}

ReturnType ASTRender::starred(Starred const* self, int depth) {
    out() << "*";
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::module(Module const* self, int depth) {
    return render_body(self->body, depth);
}

ReturnType ASTRender::raise(Raise const* self, int depth) {
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

ReturnType ASTRender::assertstmt(Assert const* self, int depth) {
    out() << special::Keyword("assert ");
    run(self->test, depth);

    if (self->msg.has_value()) {
        out() << ", ";
        run(self->msg.value(), depth);
    }
    return false;
}

ReturnType ASTRender::with(With const* self, int depth) {
    out() << special::Keyword("with ");

    int i = 0;
    for (auto const& item: self->items) {
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

ReturnType ASTRender::import(Import const* self, int depth) {
    out() << special::Keyword("import ");

    int i = 0;
    for (auto const& alias: self->names) {
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

ReturnType
ASTRender::importfrom(ImportFrom const* self, int depth) {
    out() << special::Keyword("from ");
    if (self->module.has_value()) {
        out() << self->module.value();
    }
    out() << special::Keyword(" import ");

    int i = 0;
    for (auto const& alias: self->names) {
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

ReturnType ASTRender::slice(Slice const* self, int depth) {
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

ReturnType
ASTRender::tupleexpr(TupleExpr const* self, int depth) {
    if (level == -1) {
        out() << join<ExprNode*>(", ", self->elts);
    } else {
        out() << "(" << join<ExprNode*>(", ", self->elts) << ")";
    }
    return false;
}

ReturnType ASTRender::listexpr(ListExpr const* self, int depth) {
    out() << "[" << join<ExprNode*>(", ", self->elts) << "]";
    return false;
}

ReturnType ASTRender::setexpr(SetExpr const* self, int depth) {
    out() << "{" << join<ExprNode*>(", ", self->elts) << "}";
    return false;
}

ReturnType ASTRender::dictexpr(DictExpr const* self, int depth) {
    Array<String> strs;
    strs.reserve(self->keys.size());

    for (int i = 0; i < self->keys.size(); i++) {
        strs.push_back(fmtstr("{}: {}", str(self->keys[i]), str(self->values[i])));
    }

    out() << "{" << join(", ", strs) << "}";
    return false;
}

ReturnType
ASTRender::matchvalue(MatchValue const* self, int depth) {
    run(self->value, depth);
    return false;
}

ReturnType
ASTRender::matchsingleton(MatchSingleton const* self, int depth) {
    StringStream ss;
    self->value.print(ss);
    out() << ss.str();
    return false;
}

ReturnType
ASTRender::matchsequence(MatchSequence const* self, int depth) {
    auto result = join(", ", self->patterns);
    out() << "[" << result << "]";
    return false;
}

ReturnType
ASTRender::matchmapping(MatchMapping const* self, int depth) {
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

ReturnType
ASTRender::matchclass(MatchClass const* self, int depth) {
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

ReturnType
ASTRender::matchstar(MatchStar const* self, int depth) {
    out() << "*";

    if (self->name.has_value()) {
        out() << self->name.value();
    }
    return false;
}

ReturnType ASTRender::matchas(MatchAs const* self, int depth) {
    if (self->pattern.has_value()) {
        run(self->pattern.value(), depth);
    }

    if (self->name.has_value()) {
        out() << special::Keyword(" as ") << self->name.value();
    }
    return false;
}

ReturnType ASTRender::matchor(MatchOr const* self, int depth) {
    out() << join(" | ", self->patterns);
    return false;
}

ReturnType ASTRender::ifstmt(If const* self, int depth) {
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

ReturnType ASTRender::match(Match const* self, int depth) {
    out() << special::Keyword("match ");
    run(self->subject, depth);
    out() << ":";
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    int i = 0;
    for (auto const& case_: self->cases) {
        matchcase(case_, depth + 1);

        if (i + 1 < self->cases.size()) {
            out() << newline;
        }

        i += 1;
    }
    return false;
}

ReturnType ASTRender::lambda(Lambda const* self, int depth) {
    out() << special::Keyword("lambda ");
    arguments(self->args, depth);
    out() << ": ";
    run(self->body, depth);
    return false;
}

ReturnType ASTRender::ifexp(IfExp const* self, int depth) {
    run(self->body, depth);
    out() << special::Keyword(" if ");
    run(self->test, depth);
    out() << special::Keyword(" else ");
    run(self->orelse, depth);
    return false;
}

void ASTRender::comprehension(Comprehension const& self, int depth) 
{
    out() << special::Keyword(" for ");
    run(self.target, depth);
    out() << special::Keyword(" in ");
    run(self.iter, depth);

    for (auto* expr: self.ifs) {
        out() << special::Keyword(" if ");
        run(expr, depth);
    }
}

void ASTRender::comprehensions(Array<Comprehension> const& self, int depth) 
{
    for(Comprehension const& comp: self) {
        comprehension(comp, depth);
    }
}

ReturnType ASTRender::listcomp(ListComp const* self, int depth) {
    out() << "[";
    run(self->elt, depth);

    comprehensions(self->generators, depth);

    out() << "]";
    return false;
}

ReturnType ASTRender::setcomp(SetComp const* self, int depth) {
    out() << "{";
    run(self->elt, depth);

    comprehensions(self->generators, depth);

    out() << "}";
    return false;
}

ReturnType
ASTRender::generateexpr(GeneratorExp const* self, int depth) {
    out() << "(";
    run(self->elt, depth);

    comprehensions(self->generators, depth);

    out() << ")";
    return false;
}

ReturnType ASTRender::dictcomp(DictComp const* self, int depth) {
    out() << "{";
    run(self->key, depth);
    out() << ": ";
    run(self->value, depth);

    comprehensions(self->generators, depth);
    out() << "}";
    return false;
}

ReturnType ASTRender::await(Await const* self, int depth) {
    out() << special::Keyword("await ");
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::yield(Yield const* self, int depth) {
    out() << special::Keyword("yield");
    if (self->value.has_value()) {
        out() << " ";
        run(self->value.value(), depth);
    }
    return false;
}

ReturnType
ASTRender::yieldfrom(YieldFrom const* self, int depth) {
    out() << special::Keyword("yield from ");
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::call(Call const* self, int depth) {

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
        auto const& key = self->keywords[i];

        out() << self->keywords[i].arg;
        out() << "=";

        run(key.value, depth);

        if (i < self->keywords.size() - 1)
            out() << ", ";
    }

    out() << ")";
    return false;
}

ReturnType ASTRender::constant(Constant const* self, int depth) {
    StringStream ss;
    self->value.print(ss);
    out() << ss.str();
    return false;
}

ASTRender::ExprRet
ASTRender::placeholder(Placeholder_t* self, int depth) {
    return false;
}

ReturnType
ASTRender::namedexpr(NamedExpr const* self, int depth) {
    run(self->target, depth);
    out() << " := ";
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::classdef(ClassDef const* self, int depth) {
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

    for (auto const& kw: self->keywords) {
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
        Docstring const& doc = self->docstring.value();
        out() << indentation << "\"\"\"" << doc.docstring << "\"\"\"";

        maybe_inline_comment(doc.comment, depth, LOC);
        out() << newline;
    }

    bool assign = false;
    k           = 0;
    for (auto const& stmt: self->body) {

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

ReturnType
ASTRender::functiondef(FunctionDef const* self, int depth) {
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

    out() << special::Keyword("def ") << self->name << "(";
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
        Docstring const& doc = self->docstring.value();
        
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

ReturnType ASTRender::inlinestmt(Inline const* self, int depth) {
    out() << indentation;

    int k = 0;
    for (auto const& stmt: self->body) {
        run(stmt, depth);

        if (k + 1 < self->body.size()) {
            out() << "; ";
        }

        k += 1;
    }

    return false;
}

ReturnType ASTRender::forstmt(For const* self, int depth) {
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

ReturnType ASTRender::trystmt(Try const* self, int depth) {
    out() << special::Keyword("try:");
    maybe_inline_comment(self->comment, depth, LOC);
    out() << newline;

    {
        auto _ = indent();
        render_body(self->body, depth + 1);
    }

    for (auto const& handler: self->handlers) {
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

ReturnType ASTRender::compare(Compare const* self, int depth) {
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

ReturnType ASTRender::binop(BinOp const* self, int depth) {
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

ReturnType
ASTRender::invalidstmt(InvalidStatement const* self, int depth) {
    //
    if (!self->tokens.empty()) {
        Unlex        unlex;
        StringStream ss;
        unlex.format(ss, self->tokens);
        out() << ss.str();
    }
    return false;
}

ReturnType ASTRender::boolop(BoolOp const* self, int depth) {

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

ReturnType ASTRender::unaryop(UnaryOp const* self, int depth) {
    print_op(self->op);
    out() << " ";
    run(self->operand, depth);

    return false;
}

ReturnType ASTRender::whilestmt(While const* self, int depth) {
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

ReturnType ASTRender::returnstmt(Return const* self, int depth) {
    out() << special::Keyword("return ");

    if (self->value.has_value()) {
        run(self->value.value(), depth);
    }

    return false;
}

ReturnType ASTRender::deletestmt(Delete const* self, int depth) {
    out() << special::Keyword("del ");

    for (int i = 0; i < self->targets.size(); i++) {
        run(self->targets[i], depth);

        if (i < self->targets.size() - 1)
            out() << ", ";
    }

    return false;
}

ReturnType
ASTRender::augassign(AugAssign const* self, int depth) {
    run(self->target, depth);
    print_op(self->op, true);
    out() << "= ";
    run(self->value, depth);
    return false;
}

ReturnType ASTRender::assign(Assign const* self, int depth) {
    run(self->targets[0], depth);
    out() << " = ";
    run(self->value, depth);
    return false;
}

ReturnType
ASTRender::annassign(AnnAssign const* self, int depth) {
    run(self->target, depth);
    out() << ": ";

    run(self->annotation, depth);
    if (self->value.has_value()) {
        out() << " = ";
        run(self->value.value(), depth);
    }

    return false;
}

ReturnType ASTRender::pass(Pass const* self, int depth) {
    out() << special::Keyword("pass");
    return false;
}

ReturnType ASTRender::breakstmt(Break const* self, int depth) {
    out() << special::Keyword("break");
    return false;
}

ReturnType
ASTRender::continuestmt(Continue const* self, int depth) {
    out() << special::Keyword("continue");
    return false;
}

ReturnType ASTRender::exprstmt(Expr const* self, int depth) {
    if (self->value != nullptr)
        run(self->value, depth);

    return false;
}

ReturnType ASTRender::global(Global const* self, int depth) {
    out() << special::Keyword("global ") << join(", ", self->names);
    return false;
}

ReturnType ASTRender::nonlocal(Nonlocal const* self, int depth) {
    out() << special::Keyword("nonlocal ") << join(", ", self->names);
    return false;
}

ReturnType ASTRender::arrow(Arrow const* self, int depth) {
    out() << '(' << join<ExprNode*>(", ", self->args) << ") -> ";
    out() << str(self->returns);
    return false;
}

ReturnType ASTRender::dicttype(DictType const* self, int depth) {
    out() << "Dict[";
    out() << str(self->key);
    out() << ", ";
    out() << str(self->value) << "]";
    return false;
}

ReturnType ASTRender::settype(SetType const* self, int depth) {
    out() << "Set[";
    out() << str(self->value) << "]";
    return false;
}

ReturnType ASTRender::name(Name const* self, int depth) {
    out() << self->id;
    return false;
}

ReturnType
ASTRender::arraytype(ArrayType const* self, int depth) {
    out() << "Array[";
    out() << str(self->value) << "]";
    return false;
}

ReturnType
ASTRender::tupletype(TupleType const* self, int depth) {
    out() << "Tuple[";
    out() << join<ExprNode*>(", ", self->types) << "]";
    return false;
}

ReturnType
ASTRender::builtintype(BuiltinType const* self, int depth) {
    out() << self->name;
    return false;
}

ReturnType
ASTRender::joinedstr(JoinedStr const* self, int depth) {
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

ReturnType ASTRender::formattedvalue(FormattedValue const* self,
                                     int                   depth) {
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

ReturnType
ASTRender::classtype(ClassType const* self, int depth) {
    out() << self->def->name;
    return false;
}

ReturnType ASTRender::exported(Exported const* self, int depth) {
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

void ASTRender::keyword(Keyword const& self, int depth) {
    out() << self.arg;
    if (self.value != nullptr) {
        out() << " = ";
        run(self.value, depth);
    }
}

void ASTRender::alias(Alias const& self, int depth) {
    out() << self.name;
    if (self.asname.has_value()) {
        out() << special::Keyword(" as ") << self.asname.value();
    }
}

ReturnType
ASTRender::functiontype(FunctionType const* self, int depth) {
    return false;
}

ReturnType
ASTRender::expression(Expression const* self, int depth) {
    return false;
}

ReturnType
ASTRender::interactive(Interactive const* self, int depth) {
    return false;
}

void ASTRender::withitem(WithItem const& self, int depth) {
    run(self.context_expr, depth);
    if (self.optional_vars.has_value()) {
        out() << special::Keyword(" as ");
        run(self.optional_vars.value(), depth);
    }
}

ReturnType ASTRender::comment(Comment const* n, int depth) {
    out() << "#" << n->comment;
    return false;
}

void ASTRender::arguments(Arguments const& self, int depth) {
    int i = 0;

    for (auto& arg: self.args) {
        out() << arg.arg;

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
    for (auto const& kw: self.kwonlyargs) {
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
