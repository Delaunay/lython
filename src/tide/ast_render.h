#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <flecs.h>

#include "ast/visitor.h"

namespace lython {

using BackspaceEventHandler = std::function<bool(int)>;
using InputEventHandler = std::function<bool(int, unsigned int)>;


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

#define LY_ReturnType bool



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

    BackspaceEventHandler backspace;
    InputEventHandler input;
};
struct Type {
    Type(String const& name): name(name) {}
    String name;
};
struct Editable {
    Editable(String const& name, Node* parent=nullptr): name(name), parent(parent) {}

    Editable(StringRef name, Node* parent=nullptr):
        Editable(String(name), parent)
    {}

    String name;
    Node* parent;

    BackspaceEventHandler backspace;
    InputEventHandler input;
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
    ImRect          rectangle = ImRect(ImVec2(0, 0), ImVec2(0, 0));
    String          string;
    ImColor         color = ImColor(255, 255, 255);
    ASTRenderStyle* style;
    bool            doubleclicked = false;

    bool hovered = false;
    bool held    = false;
    bool pressed = false;
};

struct DrawingRef {
    int id = -1;
    struct ASTRender* _holder = nullptr;

    Drawing* operator ->();

    operator bool() {
        return id != -1 && _holder != nullptr;
    }
};

struct Group {
    Node*                   node = nullptr;
    DrawingRef              drawing;
    int id                  = -1;
    int edit_id             = -1;
    ImRect                  rectangle = ImRect(ImVec2(0, 0), ImVec2(0, 0));
    Array<StmtNode*> const* body = nullptr;

    BackspaceEventHandler backspace;
    InputEventHandler input;
};

struct GroupRef {
    int id = -1;
    struct ASTRender* _holder = nullptr;

    Group* operator ->();

    operator bool() {
        return id != -1 && _holder != nullptr;
    }
};


struct EditableString {
    String buffer;
    String original;
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
    using StmtRet = LY_ReturnType;
    using ExprRet = LY_ReturnType;
    using ModRet  = LY_ReturnType;
    using PatRet  = LY_ReturnType;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

// Change this to return strings so we can change the format of partial results


struct ASTRender: public BaseVisitor<ASTRender, false, ASTRenderTrait> {
    using Super = BaseVisitor<ASTRender, false, ASTRenderTrait>;

    ASTRender(ASTRenderStyle* style = nullptr): style(style) {}

    ImVec2          start    = ImVec2(60, 20);
    ASTRenderStyle* style    = nullptr;
    ImVec2          cursor   = ImVec2(60, 20);
    float           maxcol   = 0;
    bool            _comment = false;
    int             level    = 0;
    int             _indent  = 0;
    bool            _redraw  = false;

    
    flecs::world& get_ecs() {
        static flecs::world ecs;
        return ecs;
    }
    Array<flecs::entity> drawings;
    Array<flecs::entity>   groups;
    Array<Node*>    stack;
    Array<int>           edit_order;
    Array<flecs::entity> entities;
    Dict<Node*, String> buffer;

    Group* new_group() {
        flecs::entity entity = get_ecs().entity()
            .add<Group>();


        entities.push_back(entity);

        Group* grp = entity.get_mut<Group>();
        groups.push_back(entity);
        grp->id = int(groups.size());
        return grp;
    }

    DrawingRef new_drawing();
    DrawingRef text(const char* name, ImColor color);

    void print_op(UnaryOperator op);
    void print_op(CmpOperator op);
    void print_op(Node* bin, BinaryOperator op, bool aug);
    void print_op(BoolOperator op);

    void draw();

    void run(Module* module) {
        ImGui::PushStyleColor(ImGuiCol_Text, style->color.Value);
        run(module, 0);
        ImGui::PopStyleColor();
    }

    template <typename T>
    Drawing* run(T* node, int depth) {
        //*
        if (node->is_leaf()) {
            stack.push_back(node);
            exec(node, depth);
            stack.pop_back();
            return nullptr;
        }//*/
        //std::cout << str(node) << std::endl;
        stack.push_back(node);
        
        int edit_entry = int(edit_order.size());
        edit_order.push_back(-1);

        int old_count = drawings.size();
        exec(node, depth);
        stack.pop_back();

        Group* group = new_group();
        group->node = node;
        int new_count    = int(drawings.size());
        group->rectangle = drawings[old_count].get_mut<Drawing>()->rectangle;
        group->edit_id = edit_entry;

        for (int i = old_count; i < new_count; i++) {
            group->rectangle.Add(drawings[i].get_mut<Drawing>()->rectangle);
        }

        edit_order[edit_entry] = group->id - 1;
        if (edit_entry != int(edit_order.size()) - 1)
        {
            edit_order.emplace_back(group->id - 1);
        }

        // if (stack.size() > 0) {Drawing
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
    ASTRender& operator<<(special::Editable const& name);

    ASTRender& out() { return *this; }

    void maybe_inline_comment(Comment* com, int depth, CodeLocation const& loc);

    LY_ReturnType render_body(Array<StmtNode*> & body, int depth, bool print_last = false);
    LY_ReturnType excepthandler(ExceptHandler & self, int depth);
    LY_ReturnType matchcase(MatchCase & self, int depth);
    void       arg(Arg & self, int depth);

    void arguments(Arguments & self, int depth);
    void withitem(WithItem & self, int depth);
    void alias(Alias & self, int depth);
    void keyword(Keyword & self, int depth);
    void comprehension(Comprehension & self, int depthh);
    void comprehensions(Array<Comprehension> & self, int depthh);

#define FUNCTION_GEN(name, fun, rtype) rtype fun(name* node, int depth);

#define X(name, _)
#define SECTION(name)
#define EXPR(name, fun)  FUNCTION_GEN(name, fun, LY_ReturnType)
#define STMT(name, fun)  FUNCTION_GEN(name, fun, LY_ReturnType)
#define MOD(name, fun)   FUNCTION_GEN(name, fun, LY_ReturnType)
#define MATCH(name, fun) FUNCTION_GEN(name, fun, LY_ReturnType)
#define VM(name, fun)

    NODEKIND_ENUM(X, SECTION, EXPR, STMT, MOD, MATCH, VM)

#undef X
#undef SECTION
#undef EXPR
#undef STMT
#undef MOD
#undef MATCH
#undef VM

#undef FUNCTION_GEN
};


}  // namespace lython