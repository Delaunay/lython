#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "ast/visitor.h"

namespace lython {
struct ASTRenderStyle {
    ImFont const* font             = nullptr;
    float         font_size        = 24;
    float         extra_line_space = 5.f;

    ImColor color     = ImColor(255, 255, 255);
    ImColor keyword   = ImColor(255, 255, 0);
    ImColor type      = ImColor(255, 0, 255);
    ImColor comment   = ImColor(30, 30, 30);
    ImColor docstring = ImColor(0, 255, 255);
};

#define ReturnType bool

namespace special {
struct Newline {};
struct Indent {};
struct BeforeComment {};
struct Keyword {
    Keyword(String const& name): name(name) {}
    String name;
};
struct Docstring {
    Docstring(String const& name): name(name) {}
    String name;
};
struct Type {
    Type(String const& name): name(name) {}
    String name;
};
}  // namespace special

struct Drawable {
    virtual ~Drawable() {}

    virtual void draw() {}
};

struct Drawing: public Drawable {
    ~Drawing() {}

    void draw();

    void input();

    int             id;
    Node*           node;
    ImRect          rectangle;
    String          string;
    ImColor         color = ImColor(255, 255, 255);
    ASTRenderStyle* style;

    bool hovered = false;
    bool held = false;
    bool pressed = false;
};

template <typename FunA, typename FunB>
struct Guard {
    FunB exit;

    Guard(FunA start, FunB end): exit(end) { start(); }

    ~Guard() { exit(); }
};

using GenericGuard = Guard<std::function<void()>, std::function<void()>>;

struct ASTRenderTrait {
    using Trace   = std::false_type;
    using StmtRet = ReturnType;
    using ExprRet = ReturnType;
    using ModRet  = ReturnType;
    using PatRet  = ReturnType;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

// Change this to return strings so we can change the format of partial results

struct ASTRender: public BaseVisitor<ASTRender, true, ASTRenderTrait> {
    using Super = BaseVisitor<ASTRender, true, ASTRenderTrait>;

    ASTRender(ASTRenderStyle* style): style(style) { }

    ImVec2           start    = ImVec2(20, 20);
    ASTRenderStyle*  style    = nullptr;
    ImVec2           cursor   = ImVec2(20, 20);
    float            maxcol   = 0;
    bool             _comment = false;
    int              level    = 0;
    int              _indent  = 0;

    
    Array<Drawing>   drawings;
    Array<Drawing*>  stack;

    Drawing* new_drawing();
    Drawing* text(const char* name, ImColor color);

    void print_op(UnaryOperator op);
    void print_op(CmpOperator op);
    void print_op(BinaryOperator op, bool aug);
    void print_op(BoolOperator op);

    void draw();

    void run(Module* module) {
        ImGui::PushStyleColor(ImGuiCol_Text, style->color.Value);
        exec(module, 0);
        ImGui::PopStyleColor();
    }

    template <typename T>
    Drawing* run(T* node, int depth) {
        // Drawing* drawing      = new_drawing();
        // drawing->node          = node;
        // drawing->rectangle.Min = cursor;
        // // drawing.color = color;
        // drawing->style = style;
        // assert(drawing.style != nullptr, "");

        // stack.push_back(&drawing);
        exec(node, depth);
        // stack.pop_back();

        // if (stack.size() > 0) {
        //     Drawing* parent = stack[stack.size() - 1];
        //     // parent->rectangle.Expand(drawing.rectangle.GetBR());
        // }

        return nullptr;
    }

    GenericGuard indent() {
        return GenericGuard([this]() { this->_indent += 1; }, [this]() { this->_indent -= 1; });
    }
    GenericGuard type() {
        return GenericGuard(
            [this]() { ImGui::PushStyleColor(ImGuiCol_Text, this->style->type.Value); },
            []() { ImGui::PopStyleColor(); });
    }

    GenericGuard comment() {
        return GenericGuard(
            [this]() { ImGui::PushStyleColor(ImGuiCol_Text, this->style->comment.Value); },
            []() { ImGui::PopStyleColor(); });
    }

    Drawable* text(Node* owner, const char* name);

    ASTRender& operator<<(const char* name);
    ASTRender& operator<<(String const& name);
    ASTRender& operator<<(StringRef const& name);
    ASTRender& operator<<(special::Indent const& name);
    ASTRender& operator<<(special::Keyword const& name);
    ASTRender& operator<<(special::Newline const& name);
    ASTRender& operator<<(special::Docstring const& name);
    ASTRender& operator<<(special::BeforeComment const& name);

    ASTRender& out() { return *this; }

    void maybe_inline_comment(Comment* com, int depth, CodeLocation const& loc);

    ReturnType render_body(Array<StmtNode*> const& body, int depth, bool print_last = false);
    ReturnType excepthandler(ExceptHandler const& self, int depth);
    ReturnType matchcase(MatchCase const& self, int depth);
    void       arg(Arg const& self, int depth);

    void arguments(Arguments const& self, int depth);
    void withitem(WithItem const& self, int depth);
    void alias(Alias const& self, int depth);
    void keyword(Keyword const& self, int depth);
    void comprehension(Comprehension const& self, int depthh);
    void comprehensions(Array<Comprehension> const& self, int depthh);

#define FUNCTION_GEN(name, fun, rtype) rtype fun(const name* node, int depth);

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

}  // namespace lython