#include "node.h"

#include "imgui_internal.h"

const float  NODE_SLOT_RADIUS = 6.0f;
const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

void GraphEditor::draw() {
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
    ImGui::Begin("GraphEditor");

    const ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;

    ImGui::BeginGroup();
    ImGui::BeginChild("scrolling_region",
                      ImVec2(0, 0),
                      true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    drawgrid();

    ImGui::PushItemWidth(120.0f);
    draw_list->ChannelsSplit(2);
    for (Forest& forest: forests) {
        for (Tree& tree: forest.trees) {
            current_tree = &tree;

            for (GraphNode& node: tree.nodes) {
                draw(&node, offset);
            }

            for (Link& link: tree.links) {
                draw(&link, offset);
            }

            handle_events(offset);
            current_tree = nullptr;
        }
    }
    draw_list->ChannelsMerge();

    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
        scrolling = scrolling + io.MouseDelta;

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::EndGroup();
    ImGui::End();

    
}

void GraphEditor::handle_events(ImVec2 offset) {
    if (current_tree == nullptr) {
        return;
    } 
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (selected_pin != nullptr && hovered_pin != nullptr) {
            current_tree->links.emplace_back(selected_pin, hovered_pin);
        }
        selected_pin = nullptr;
        hovered_pin  = nullptr;
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (selected_pin != nullptr) {
            draw_bezier(draw_list,
                        selected_pin->pos + offset,
                        ImGui::GetMousePos(),
                        IM_COL32(200, 200, 100, 255));
        }
    }
}

void GraphEditor::drawgrid() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Display grid
    if (show_grid) {
        ImU32  GRID_COLOR = IM_COL32(200, 200, 200, 40);
        float  GRID_SZ    = 64.0f;
        ImVec2 win_pos    = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz  = ImGui::GetWindowSize();

        for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
            draw_list->AddLine(
                ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);

        for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
            draw_list->AddLine(
                ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
    }
}

uint64_t nextid() {
    static uint64_t counter = 0;
    return counter++;
}

void draw_bezier(ImDrawList* draw_list, ImVec2 p1, ImVec2 p2, ImU32 color) {
    if (p1.x > p2.x) {
        std::swap(p1, p2);
    }

    auto sgn = [](float v) -> int { return int(v > 0); };

    auto offset   = ImVec2((p2.x - p1.x + 1) / 2.f, 0);
    auto y_offset = sgn(p2.y - p1.y) * (p2.y - p1.y) / 2.f;

    if (p2.y - p1.y > p2.x - p1.x) {
        offset = ImVec2(0, y_offset);
    }

    draw_list->AddBezierCubic(p1, p1 + offset, p2 - offset, p2, color, 1);
}

void GraphEditor::draw(Link* link, ImVec2 offset) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_bezier(
        draw_list, link->from->pos + offset, link->to->pos + offset, IM_COL32(200, 200, 100, 255));
}

void GraphEditor::draw(GraphNode* node, ImVec2 offset) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO&    io        = ImGui::GetIO();

    ImGui::PushID(node->id);

    ImVec2       node_rect_min = offset + node->pos;  // Parent offset
    static float value;
    bool         old_any_active = ImGui::IsAnyItemActive();

    // Content
    draw_list->ChannelsSetCurrent(1);
    ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
    ImGui::BeginGroup();
    const char* name = node->name.c_str();
    ImGui::Text("%s", name);
    ImGui::SliderFloat("##value", &value, 0.0f, 1.0f, "Alpha %.2f");
    ImGui::EndGroup();
    // ---
    node->size           = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
    ImVec2 node_rect_max = node_rect_min + node->size;

    // Background
    draw_list->ChannelsSetCurrent(0);
    ImGui::SetCursorScreenPos(node_rect_min);
    ImGui::InvisibleButton("node", node->size);
    /// ---

    // Events
    if (ImGui::IsItemHovered()) {
        hovered_node = node;
        open_context_menu |= ImGui::IsMouseClicked(1);
    }

    bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
    bool node_moving_active  = ImGui::IsItemActive();
    if (node_widgets_active || node_moving_active)
        selected_node = node;

    if (node_moving_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        node->pos = node->pos + io.MouseDelta;
    // ----

    ImU32 node_bg_color = IM_COL32(75, 75, 75, 255);

    draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
    draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(100, 100, 100, 255), 4.0f);

    ImVec2 Pos          = node->pos;
    ImVec2 Size         = node->size;
    int    InputsCount  = node->inputs.size();
    int    OutputsCount = node->outputs.size();

    auto GetInputSlotPos = [=](int slot_no) -> ImVec2 {
        return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1));
    };
    auto GetOutputSlotPos = [=](int slot_no) -> ImVec2 {
        return ImVec2(Pos.x + Size.x,
                      Pos.y + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1));
    };

    auto DrawPin = [=](ImVec2 center, Pin& pin) {
        ImGui::PushID(pin.id);
        // ImGui::SetCursorScreenPos(center - ImVec2(1, 1) * NODE_SLOT_RADIUS);

        ImVec2 radius = ImVec2(1, 1) * NODE_SLOT_RADIUS;
        ImRect bb(center - radius, center + radius);

        bool hovered = false;
        bool held    = false;
        auto flags   = ImGuiButtonFlags_PressedOnClick;

        ImGui::ButtonBehavior(bb, pin.id, &hovered, &held, flags);

        if (held) {
            selected_pin = &pin;
        }

        if (hovered && &pin != selected_pin) {
            hovered_pin = &pin;
        }

        draw_list->AddCircleFilled(center, NODE_SLOT_RADIUS, IM_COL32(150, 150, 150, 150));

        ImGui::PopID();
    };

    // Pins
    // ----
    for (int slot_idx = 0; slot_idx < node->inputs.size(); slot_idx++) {
        Pin& pin      = node->inputs[slot_idx];
        pin.pos       = GetInputSlotPos(slot_idx);
        ImVec2 center = offset + pin.pos;
        DrawPin(center, pin);
    }

    for (int slot_idx = 0; slot_idx < node->outputs.size(); slot_idx++) {
        Pin& pin      = node->outputs[slot_idx];
        pin.pos       = GetOutputSlotPos(slot_idx);
        ImVec2 center = offset + pin.pos;
        DrawPin(center, pin);
    }
    // ---

    ImGui::PopID();
}