#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <string>
#include <unordered_map>
#include <vector>

uint64_t nextid();
bool     draw_bezier(ImDrawList* draw_list,
                     ImVec2      p1,
                     ImVec2      p2,
                     ImU32       color,
                     int         segment,
                     float       tickness = 2,  //
                     float       eps      = 2   //
    );

struct Base {
    uint64_t id;

    Base(): id(nextid()) {}
};

enum class PinKind
{
    Flow,
    Circle,
    Square,
    Grid,
    RoundSquare,
    Diamond,
};

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Delegate,
};

struct Pin: public Base {
    std::string name;
    // std::string typename;
    PinType type = PinType::Object;
    PinKind kind = PinKind::Circle;
    ImVec2  pos;
    bool    connected = false;

    // Inline value
    uint64_t value;
    float* as_float() {
        return reinterpret_cast<float*>(&value);
    }
};

struct Link {
    Link(Pin* from, Pin* to): from(from), to(to) {
        from->connected = true;
        to->connected   = true;
    }

    std::string name;
    
    Pin* from;
    Pin* to;
};

struct Layout {
    ImVec2 input  = ImVec2(120, 50);
    ImVec2 output = ImVec2(120, 50);
};

// Expression
struct Node: public Base {
    ImVec2      pos;
    ImVec2      size;
    std::string name;
    bool selected = false;

    std::vector<Pin> inputs;
    std::vector<Pin> outputs;

    Layout layout;

    // -----
    Pin* exec_in() {
        for(Pin& p : inputs) {
            if (p.type == PinType::Flow)
                return &p;
        }
        return nullptr;
    }

    Pin* exec_out() {
        for(Pin& p : outputs) {
            if (p.type == PinType::Flow)
                return &p;
        }
        return nullptr;
    }
};

// Statement
// Function
// Macro
struct Tree: public Base {
    std::string name;

    std::vector<Node> nodes;
    std::vector<Link> links;
};

// module
// namespace
struct Forest: public Base {

    std::vector<Tree> trees;
};

struct GraphEditor: public Base {

    std::vector<Forest> forests;

    void draw();

    void draw(Pin* pin, ImVec2 center);
    void draw(Link* link, ImVec2 offset);
    void draw(Node* node, ImVec2 offset);
    void drawgrid();

    void handle_events(ImVec2 offset);

    //
    ImVec2                             scrolling     = ImVec2(0.0f, 0.0f);
    Node*                              hovered_node  = nullptr;
    Node*                              selected_node = nullptr;
    Pin*                               hovered_pin   = nullptr;
    Pin*                               selected_pin  = nullptr;
    Link*                              hovered_link  = nullptr;
    Tree*                              selected_tree = nullptr;
    Tree*                              current_tree  = nullptr;
    std::unordered_map<PinType, ImU32> _colors       = {{
        {PinType::Flow, IM_COL32(255, 255, 255, 255)},
        {PinType::Bool, IM_COL32(220, 48, 48, 255)},
        {PinType::Int, IM_COL32(68, 201, 156, 255)},
        {PinType::Float, IM_COL32(147, 226, 74, 255)},
        {PinType::String, IM_COL32(124, 21, 153, 255)},
        {PinType::Object, IM_COL32(51, 150, 215, 255)},
        {PinType::Delegate, IM_COL32(255, 48, 48, 255)},
    }};

    // State
    float pin_label_margin   = 5;
    float node_padding       = 4.0f;
    float pin_radius         = 12;
    int   bezier_segments    = 10;
    float tickness           = 3;
    bool  show_grid          = true;
    bool  open_context_menu  = false;
    ImU32 node_bg_color      = IM_COL32(75, 75, 75, 255);
    ImU32 node_outline_color = IM_COL32(100, 100, 100, 255);
    ImU32 node_selected_color= IM_COL32( 50,  50, 200, 255);
    ImU32 rectangle_color    = IM_COL32(100, 100, 200, 125);
    bool  rectangle_select   = false;
    ImVec2 rectangle_start;
    ImRect rectangle_selection;

    std::unordered_map<Node*, bool>  selected;

    ImRect _size;
    ImVec2 _offset;

    ImVec2 get_offset() { return _offset; }

    ImVec2 estimate_size(Node& node, ImVec2 font_size = ImVec2(7, 13));

    bool is_selecting() const {
        return rectangle_select;
    }
    ImVec2 with_scroll(ImVec2 pos) {
        return pos + _offset;
    }

    bool has_selection() const {
        return selected.size() > 0;
    }

    ImRect selection_rectangle() const {
        return rectangle_selection;
    } 

    void check_selected(Node* node) {
        if (is_selecting()) {
            if (selection_rectangle().Contains(ImRect(node->pos, node->size))) {
                node->selected = true;
                selected[node] = true;
            }    
            else {
                node->selected = false;
                selected.extract(node);
            }
        }
    }

    private:
    struct PinStyle {
        PinKind kind;
        bool    filled;
        ImU32   color;
        ImU32   fill;
    };

    void draw(PinStyle& style, ImVec2 pos, ImVec2 size);
    void draw_flow(PinStyle& style, ImVec2 pos, ImVec2 size);
    void draw_circle(PinStyle& style, ImVec2 pos, ImVec2 size);
    void draw_square(PinStyle& style, ImVec2 pos, ImVec2 size);
    void draw_grid(PinStyle& style, ImVec2 pos, ImVec2 size);
    void draw_round_square(PinStyle& style, ImVec2 pos, ImVec2 size);
    void draw_diamond(PinStyle& style, ImVec2 pos, ImVec2 size);
    void draw_triangle(PinStyle& style, ImVec2 pos, ImVec2 size);
};



struct TreeBuilder {
    TreeBuilder(Tree& t):
        tree(t)
    {}

    Node& mew_node() {
        Node& n = tree.nodes.emplace_back();
        return n;
    }

    Tree& tree;
};

struct GraphBuilder {
    GraphBuilder(GraphEditor& ed):
        editor(ed)
    {}



    GraphEditor& editor;
};
