#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "ast/visitor.h"


namespace lython {
struct ASTRenderStyle {
    ImFont const* font = nullptr;
    float font_size = 24;
    float extra_line_space = 5.f;

    ImColor color = ImColor(255, 255, 255);
    ImColor keyword = ImColor(255, 255, 0);
    ImColor type = ImColor(255, 0, 255);
    ImColor comment = ImColor(30, 30, 30);
    ImColor docstring = ImColor(0, 255, 255);
};



#define ReturnType bool

namespace special {
    struct Newline {};
    struct Indent {};
    struct BeforeComment {};
    struct Keyword {
        Keyword(String const& name):
            name(name)
        {}
        String name;
    };
    struct Docstring {
        Docstring(String const& name):
            name(name)
        {}
        String name;
    };
    struct Type {
        Type(String const& name):
            name(name)
        {}
        String name;
    };
}

struct ASTRenderContext {
    //
    ImVec2          start = ImVec2(10, 10);
    ASTRenderStyle* style = nullptr;
    ImVec2          pos = ImVec2(10, 10);
    float           maxcol = 0;
    int             _indent = 0;
    bool            _comment = false;
    unsigned int    _id = 1;

    void inline_string(const char* str, ImColor color);

    ASTRenderContext& operator<< (special::Docstring const& str);
    ASTRenderContext& operator<< (special::Type const& str);
    ASTRenderContext& operator<< (special::Indent const& str);
    ASTRenderContext& operator<< (special::BeforeComment const& str);
    ASTRenderContext& operator<< (special::Newline const& str);
    ASTRenderContext& operator<< (special::Keyword const& str);
    ASTRenderContext& operator<< (const char* str);
    ASTRenderContext& operator<< (String const& str);

    template<typename T>
    ASTRenderContext& operator<< (T const& value) {
        StringStream ss;
        ss << value;
        return (*this) << ss.str();
    }

    template<typename FunA, typename FunB>
    struct Guard {
        FunB exit;

        Guard(FunA start, FunB end):
            exit(end)
        {
            start();
        }

        ~Guard() {
            exit();
        }
    };

    using GenericGuard = Guard<std::function<void()>, std::function<void()>>;

    GenericGuard type() {
        return GenericGuard(
            [this]() {
                ImGui::PushStyleColor(ImGuiCol_Text, this->style->type.Value);
            }, 
            [](){
                ImGui::PopStyleColor();
            }
        );
    }

    GenericGuard comment() {
        return GenericGuard(
            [this]() {
                ImGui::PushStyleColor(ImGuiCol_Text, this->style->comment.Value);
            }, 
            [](){
                ImGui::PopStyleColor();
            }
        );
    }

    GenericGuard indent() {
        return GenericGuard(
            [this]() {
                this->_indent += 1;
            }, 
            [this](){
                this->_indent -= 1;
            }
        );
    }
};


struct ASTRenderTrait {
    using Trace   = std::false_type;
    using StmtRet = bool;
    using ExprRet = bool;
    using ModRet  = bool;
    using PatRet  = bool;

    enum
    { MaxRecursionDepth = LY_MAX_VISITOR_RECURSION_DEPTH };
};

// Change this to return strings so we can change the format of partial results

struct ASTRender: public BaseVisitor<ASTRender, true, ASTRenderTrait, ASTRenderContext&, int> {
    using Super = BaseVisitor<ASTRender, true, ASTRenderTrait, ASTRenderContext&, int>;

    ASTRender(ASTRenderStyle* style):
        style(style)
    {
        context.style = style;
    }

    ASTRenderStyle*  style;
    ASTRenderContext context;

    void run(Module* module) {
        ImGui::PushStyleColor(ImGuiCol_Text, style->color.Value);
        exec(module, 0, context, 0);
        ImGui::PopStyleColor();
    }

    void maybe_inline_comment(Comment* com, int depth, ASTRenderContext& out, int level, CodeLocation const& loc);

    ReturnType render_body(Array<StmtNode*> const& body,
                        int                     depth,
                        ASTRenderContext&           out,
                        int                     level,
                        bool                    print_last = false);
    ReturnType excepthandler(ExceptHandler const& self, int depth, ASTRenderContext& out, int level);
    ReturnType matchcase(MatchCase const& self, int depth, ASTRenderContext& out, int level);
    void arg(Arg const& self, int depth, ASTRenderContext& out, int level);

    void arguments(Arguments const& self, int depth, ASTRenderContext& out, int level);
    void withitem(WithItem const& self, int depth, ASTRenderContext& out, int level);
    void alias(Alias const& self, int depth, ASTRenderContext& out, int level);
    void keyword(Keyword const& self, int depth, ASTRenderContext& out, int level);
    void comprehension(Comprehension const& self, int depthh, ASTRenderContext& out, int level);
    void comprehensions(Array<Comprehension> const& self, int depthh, ASTRenderContext& out, int level);

#define FUNCTION_GEN(name, fun, rtype) \
    rtype fun(const name* node, int depth, ASTRenderContext& out, int level);

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

}