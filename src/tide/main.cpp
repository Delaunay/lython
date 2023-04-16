#include <vk_engine.h>

// #include "graphed/graphed.h"
#include "imgui_impl_vulkan.h"

#include "node.h"

void ShowExampleAppCustomNodeGraph(bool* opened);

class App: public VulkanEngine {
    public:
    GraphEditor editor;

    App() {}

    void start() {
        Forest& forest = editor.forests.emplace_back();
        Tree& tree = forest.trees.emplace_back();

        Node& n1 = tree.nodes.emplace_back();
        n1.pos = ImVec2(10, 10);
        n1.name = "First Node";

		{
			Pin& p = n1.inputs.emplace_back();
			p.kind = PinKind::Flow;
			p.type = PinType::Flow;
		}

        {	
			Pin& p = n1.inputs.emplace_back();
			p.name = "x";
			p.type = PinType::Float;
		}

		{	
			Pin& p = n1.inputs.emplace_back();
			p.name = "y";
		}

		{
			Pin& p = n1.outputs.emplace_back();
			p.kind = PinKind::Flow;
			p.type = PinType::Flow;
		}

		Pin& p1 = n1.outputs.emplace_back();
		p1.name = "z";

		// -- NODE 2
        Node& n2 = tree.nodes.emplace_back();
        n2.pos = ImVec2(300, 100);
        n2.name = "+";
        Pin& p2 = n2.inputs.emplace_back();
		p2.name = "b";

        {
			Pin& p = n2.outputs.emplace_back();
			p.name = "a";
		}
        
		{
			Pin& p = n2.outputs.emplace_back();
			p.name = "c";
		}

		tree.links.emplace_back(&p1, &p2);		
    }

    void handle_event(SDL_Event const& event) {}

    void tick(float dt) {
        editor.draw();

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
