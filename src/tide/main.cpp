#include "tide/convert/to_graph.h"

#include <vk_engine.h>

// #include "graphed/graphed.h"
#include "imgui_impl_vulkan.h"

#include "node.h"
#include "block.h"

void ShowExampleAppCustomNodeGraph(bool* opened);

using namespace lython;

class App: public VulkanEngine {
    public:
    GraphEditor editor;
    Arena arena;

    App() {}

    using TNode = lython::GraphNode;
    using TPin = lython::GraphNodePin;


    TPin* new_pin(TNode* n , const char* name, PinDirection dir, PinType tp, PinKind k) {
        TPin* p = arena.new_object<TPin>();
        n->pins().push_back(p);
        p->name() = name;
        p->direction() = dir;
        p->type() = tp;
        p->kind() = k;
        return p;
    }

    void start() {
        Forest& forest = editor.forests.emplace_back();
        Tree& tree = forest.trees.emplace_back();

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
        }*/

        /*/ Condition
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
        } */
        
    }

    void handle_event(SDL_Event const& event) {}

    void tick(float dt) {
        editor.draw();

        /*
        FlowBlock b;
        b.draw();
        */
    
        //bool opened = true;
        //ShowExampleAppCustomNodeGraph(&opened);
    }

    void end() {
    }
};

#include <iostream>

int main(int argc, char* argv[]) 
{
    App engine;

    engine.init();

    engine.run();

    engine.cleanup();

    return 0;
}
