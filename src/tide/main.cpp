#include "tide/ast_render.h"
#include "tide/convert/to_graph.h"

#include <SDL_ttf.h>
#include <vk_engine.h>

// #include "graphed/graphed.h"
#include "convert/to_graph.h"
#include "imgui_impl_vulkan.h"

#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

#include "block.h"
#include "convert/to_graph.h"
#include "lexer/buffer.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "node.h"
#include "parser/parser.h"

void ShowExampleAppCustomNodeGraph(bool* opened);

using namespace lython;

String code = "a = 2\n"
              "b = 1\n"
              "if a > 1:\n"
              "    b = a + 2\n"
              "c = a + b\n";

class App: public VulkanEngine {
    public:
    GraphEditor editor;
    Arena       arena;
    Module*     module;

    App() {}

    using TNode = lython::GraphNode;
    using TPin  = lython::GraphNodePin;

    TPin* new_pin(TNode* n, const char* name, PinDirection dir, PinType tp, PinKind k) {
        TPin* p = arena.new_object<TPin>();
        n->pins().push_back(p);
        p->name()      = name;
        p->direction() = dir;
        p->type()      = tp;
        p->kind()      = k;
        return p;
    }

    void start() {

#define CODE(X) #X
        String       code = "def func(a: int, b: int) -> int:    # //\n"
                            "    \"\"\"This is a docstring\"\"\"\n"
                            "    return a + b                    # //\n";
        StringBuffer reader(code);
        Lexer        lex(reader);
        Parser       parser(lex);

        module = parser.parse_module();

        // TTF_Init();
        // // TTF_Quit();
        // Font            = TTF_OpenFont("K:/lython/assets/Red_Hat_Mono/RedHatMono-Regular.ttf",
        // 50); SDL_Color Color = {20, 20, 20}; TextSurface     = TTF_RenderUTF8_Blended(Font,
        // "hello", Color);

        // TTF_CloseFont(Font);

        /*
        Forest& forest = editor.forests.emplace_back();
        Tree& tree = forest.trees.emplace_back();

        /*
        StringBuffer reader("a = 2 + 1");
        Lexer  lex(reader);
        Parser parser(lex);
        Module* m = parser.parse_module();
        // ToGraph().exec(m);
        * /

        //
        StringBuffer reader(code);
        Lexer  lex(reader);
        Parser parser(lex);

        auto mod = Unique<Module>(parser.parse_module());

        // Add the graph to the forest
        auto converter = ToGraph();
        converter.exec(mod.get(), 0);
        //

        TNode* n1 = arena.new_object<TNode>();

                {

            tree.nodes.push_back(n1);
            n1->position() = ImVec2(10, 10);
            n1->name() = "First Node";

            new_pin(n1, "", PinDirection::Input, PinType::Flow, PinKind::Flow);
            new_pin(n1, "x", PinDirection::Input, PinType::Float, PinKind::Circle);
            new_pin(n1, "y", PinDirection::Input, PinType::Float, PinKind::Circle);

            new_pin(n1, "", PinDirection::Output, PinType::Flow, PinKind::Flow);
            new_pin(n1, "z", PinDirection::Output, PinType::Float, PinKind::Circle);

                }

        ImVec2 s1 = editor.estimate_size(n1);

        TNode* n2 = arena.new_object<TNode>();
        {
            tree.nodes.push_back(n2);

            n2->position() = s1 * ImVec2(1, 0) + ImVec2(40, 10);
            n2->name() = "+";

            new_pin(n2, "x", PinDirection::Input, PinType::Float, PinKind::Circle);
            new_pin(n2, "y", PinDirection::Input, PinType::Float, PinKind::Circle);
            new_pin(n2, "z", PinDirection::Output, PinType::Float, PinKind::Circle);
        }


        /*
        TNode* n3 = arena.new_object<TNode>();
        {
            tree.nodes.push_back(n3);
            n3->position() = ImVec2(600, 100);
            n3->name() = "Pure Function";

            new_pin(n2, "x", PinDirection::Output, PinType::Object, PinKind::Circle);
        }* /

        /* / Condition
        {
            Node& n = tree.nodes.emplace_back();
            n.pos = ImVec2(600, 100);
            n.name = "Cond";

            {
                Pin& p = n.inputs.emplace_back();
                p.kind = PinKind::Flow;
                p.type = PinType::Flow;
            }

            {
                Pin& p = n.outputs.emplace_back();
                p.kind = PinKind::Flow;
                p.type = PinType::Flow;
                p.name = "True";
            }

            {
                Pin& p = n.outputs.emplace_back();
                p.kind = PinKind::Flow;
                p.type = PinType::Flow;
                p.name = "False";
            }
        } * /

        */
    }

    void handle_event(SDL_Event const& event) {}

    void tick(float dt) {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::Begin(
            "WorkSpace", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

        // ImGui::Text("Here");
        // ImDrawList* drawlist = ImGui::GetWindowDrawList();
        // drawlist->AddText(ImVec2(10, 10), ImColor(255, 255, 255), "Hello");

        ASTRenderStyle style;
        style.font = ImGui::GetFont();
        ASTRender render(&style);
        render.run(module);

        ImGui::End();
        ImGui::PopStyleVar(1);
    }

    void end() {}
};

#include <iostream>

int SDL_main(int argc, char* argv[]) {
    std::cout << "HERE\n";

    App engine;

    engine.init();

    engine.run();

    engine.cleanup();

    return 0;
}
