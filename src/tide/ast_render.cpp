#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

#include "tide/ast_input.h"
#include "tide/ast_render.h"

#include "utilities/printing.h"
#include "ast/nodes.h"
#include "ast/ops.h"
// #include "ast/values/native.h"
// #include "ast/values/object.h"
#include "ast/visitor.h"
#include "dependencies/fmt.h"
#include "lexer/unlex.h"
#include "logging/logging.h"
#include "parser/parsing_error.h"
#include "utilities/allocator.h"
#include "utilities/strings.h"

bool ASTInputText(const char*            label,
                  const char*            hint,
                  char*                  buf,
                  int                    buf_size,
                  const ImVec2&          size_arg,
                  ImGuiInputTextFlags    flags,
                  ImGuiInputTextCallback callback,
                  void*                  callback_user_data);

#define LOG(X) std::cout << X << std::endl;

namespace lython {


Drawing* DrawingRef::operator ->() {
    return _holder->drawings[id].get_mut<Drawing>();
}

Group* GroupRef::operator ->() {
    return _holder->drawings[id].get_mut<Group>();
}

void Drawing::draw() {
    ImDrawList* drawlist = ImGui::GetWindowDrawList();

    if (string.empty()) {
        drawlist->AddRect(rectangle.Min, rectangle.Max, color);
    }
    else {
        const char* str = string.c_str();
        auto color_1 = color;

        if (string[0] == '<') {
            color_1 = ImColor(25, 25, 25);
        }
        drawlist->AddText(
            style->font,
            style->font_size + hovered,
            rectangle.GetTL(),
            color_1,
            str
        );
    }
};

void Drawing::input() {
    ImGui::ItemAdd(rectangle, id);
    pressed = ImGui::ButtonBehavior(rectangle, id, &hovered, &held, 0);

    // ImGuiMouseButton button = ImGuiMouseButton_Left;
    // if (hovered && ImGui::IsMouseDoubleClicked(button)) {
    //     doubleclicked = true;
    // }
}

DrawingRef ASTRender::new_drawing() {
    flecs::entity entity = get_ecs().entity().add<Drawing>();
    entities.push_back(entity);

    Drawing* drawing = entity.get_mut<Drawing>();
    drawings.push_back(entity);

    drawing->id       = drawings.size();
    drawing->style    = style;
    drawing->node     = stack[stack.size() - 1];

    return DrawingRef{drawing->id - 1, this};
}

DrawingRef ASTRender::text(const char* name, ImColor color) {

    if (strcmp(name, "") == 0) {
        name = "<missing>";
    }

    DrawingRef drawing = new_drawing();
    drawing->string  = name;
    drawing->color   = color;

    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, name);

    drawing->rectangle = ImRect(cursor, cursor + size);
    cursor.x += size.x;
    return drawing;
}

void ASTRender::draw() {
    for (flecs::entity& entity: drawings) {
        entity.get_mut<Drawing>()->input();
        entity.get_mut<Drawing>()->draw();
    }
}

static special::Newline       newline;
static special::Indent        indentation;
static special::BeforeComment comment_space;

ASTRender& ASTRender::operator<<(String const& name) { return (*this) << name.c_str(); }
ASTRender& ASTRender::operator<<(StringRef const& name) { return (*this) << String(name).c_str(); }

ASTRender& ASTRender::operator<<(const char* name) {
    text(name, style->color);
    return (*this);
}

ASTRender& ASTRender::operator<<(special::Indent const& name) {
    String idt(4 * _indent, ' ');
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, idt.c_str());
    cursor += ImVec2(size.x, 0);
    return *this;
}
ASTRender& ASTRender::operator<<(special::Keyword const& keyword) {
    text(keyword.name.c_str(), style->keyword);
    return *this;
}
ASTRender& ASTRender::operator<<(special::Newline const& name) {
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, " ");

    // if (!_comment) {
    //     maxcol = std::max(cursor.x, maxcol);
    // }
    // _comment = false;

    cursor.x = start.x;
    cursor.y += (size.y + style->extra_line_space);
    return *this;
}
ASTRender& ASTRender::operator<<(special::Docstring const& keyword) {
    DrawingRef drawing       = new_drawing();
    drawing->color         = style->comment;
    drawing->string        = keyword.name.empty() ? String("<docstring>") : keyword.name;
    drawing->rectangle.Min = cursor;

    ImVec2 size =
        style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, drawing->string.c_str());

    drawing->rectangle = ImRect(cursor, cursor + size);

    Group* group = new_group();
    group->drawing = drawing;
    group->edit_id = edit_order.size();
    edit_order.push_back(group->id - 1);
    group->rectangle = drawing->rectangle;
    group->input = keyword.input;
    group->backspace = keyword.backspace;

    cursor.x += size.x;
    return *this;
}
ASTRender& ASTRender::operator<<(special::BeforeComment const& name) {
    String idt  = "   ";
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, idt.c_str());

    float maybe_x = cursor.x + size.x;
    maxcol        = std::max(maybe_x, maxcol);

    cursor.x = maxcol;

    _comment = true;
    return *this;
}

ASTRender& ASTRender::operator<<(special::Editable const& name)
{
    DrawingRef drawing = text(name.name.c_str(), style->color);

    Group* group = new_group();
    group->node = name.parent;
    group->drawing = drawing;
    group->edit_id = edit_order.size();
    edit_order.push_back(group->id - 1);
    group->rectangle = drawing->rectangle;
    group->backspace = name.backspace;
    group->input = name.input;
    return (*this);
}

void ASTRender::maybe_inline_comment(Comment* com, int depth, CodeLocation const& loc) {
    if (com != nullptr) {
        // lython::log(lython::LogLevel::Info, loc, "printing inline comment {}", com->comment);
        out() << comment_space;
        {
            auto _ = comment();
            run(com, depth);
        }
    }
}

}