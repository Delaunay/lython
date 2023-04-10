#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <string>
#include <vector>

uint64_t nextid();
void draw_bezier(ImDrawList* draw_list, ImVec2 p1, ImVec2 p2, ImU32 color);

struct Base {
    uint64_t id;

    Base(): id(nextid()) {}
};


struct Pin: public Base {
    std::string name;
    std::string type;
    std::string kind;
    ImVec2      pos;
};

struct Link {
    Link(Pin* from, Pin* to):
        from(from), to(to)
    {}

    Pin* from;
    Pin* to;
};


// Expression
struct GraphNode: public Base {
    ImVec2      pos;
    ImVec2      size;
    std::string name;

    std::vector<Pin> inputs;
    std::vector<Pin> outputs;
};

// Statement
// Function
// Macro
struct Tree: public Base {
    std::string name;

    std::vector<GraphNode> nodes;
    std::vector<Link>      links;
};

// module
// namespace
struct Forest: public Base {

    std::vector<Tree> trees;
};

struct GraphEditor: public Base {

    std::vector<Forest> forests;

    void draw();

    void draw(Link* link, ImVec2 offset);
    void draw(GraphNode* node, ImVec2 offset);
    void drawgrid();

    void handle_events(ImVec2 offset);

    //
    ImVec2   scrolling = ImVec2(0.0f, 0.0f);
    GraphNode* hovered_node = nullptr;
    GraphNode* selected_node = nullptr;

    Pin* hovered_pin = nullptr;
    Pin* selected_pin = nullptr;

    Tree*    current_tree      = nullptr;
    bool     show_grid         = true;
    bool     open_context_menu = false;
};