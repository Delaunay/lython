#include <vk_engine.h>

// #include "graphed/graphed.h"
#include "imgui_impl_vulkan.h"

#include "node.h"

void ShowExampleAppCustomNodeGraph(bool* opened);

class App: public VulkanEngine {
    public:
    // GraphEd Editor;

    GraphEditor editor;

    App() {}

    void start() {
        // Editor.OnStart();

        Forest& forest = editor.forests.emplace_back();
        Tree& tree = forest.trees.emplace_back();

        GraphNode& n1 = tree.nodes.emplace_back();
        n1.pos = ImVec2(0, 0);
        n1.name = "First Node";
        n1.inputs.emplace_back();
        n1.inputs.emplace_back();
        n1.outputs.emplace_back();

        GraphNode& n2 = tree.nodes.emplace_back();
        n2.pos = ImVec2(10, 10);
        n2.name = "Second Node";
        n2.inputs.emplace_back();
        n2.outputs.emplace_back();
        n2.outputs.emplace_back();
    }

    void handle_event(SDL_Event const& event) {}

    void tick(float dt) {
        // ImGui::Begin("EditorFrame");
        // Editor.OnFrame(dt);
        // ImGui::End();

        ImGui::Begin("EditorFrame");
        editor.draw();
        ImGui::End();

        bool opened = true;
        ShowExampleAppCustomNodeGraph(&opened);
    }

    void end() {
        // Editor.OnStop();
    }
};

int main(int argc, char* argv[]) {
    App engine;

    engine.init();

    engine.run();

    engine.cleanup();

    return 0;
}
