#include "node.h"
#include "bezier.h"

#include "imgui_internal.h"

#include <algorithm>


namespace lython {

void GraphEditor::draw() {
    hovered_link = nullptr;
    hovered_pin  = nullptr;

    ImGuiIO& io = ImGui::GetIO();

    // ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
#ifdef IMGUI_HAS_VIEWPORT
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetWorkPos());
    ImGui::SetNextWindowSize(viewport->GetWorkSize());
    ImGui::SetNextWindowViewport(viewport->ID);
#else
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("GraphEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    const ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;
    _offset             = offset;

    ImGui::BeginGroup();
    ImGui::BeginChild("scrolling_region",
                      ImVec2(0, 0),
                      true,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    drawgrid();

    ImGui::PushItemWidth(120.0f);
    draw_list->ChannelsSplit(3);
    for (Forest& forest: forests) {
        for (Tree& tree: forest.trees) {
            current_tree = &tree;

            for (GraphNodeBase* node: tree.nodes) {
                draw(node, offset);
            }

            draw_list->ChannelsSetCurrent(0);
            for (Link& link: tree.links) {
                draw(&link, offset);
            }

            current_tree = nullptr;
        }
    }
    draw_list->ChannelsMerge();

    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() &&
        ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
        scrolling = scrolling + io.MouseDelta;

    handle_events(offset);

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::EndGroup();
    ImGui::End();

    ImGui::PopStyleVar(1);
}

void GraphEditor::handle_events(ImVec2 offset) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // REMOVE LINK
    if (ImGui::IsKeyDown(ImGuiKey_LeftAlt) && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (hovered_link != nullptr) {
            auto it = std::remove_if(std::begin(selected_tree->links),
                                     std::end(selected_tree->links),
                                     [=](Link const& link) {
                                         Link const* l = &link;
                                         return l == hovered_link;
                                     });

            //it->from->connected = false;
            //it->to->connected   = false;

            selected_tree->links.erase(it);

            hovered_link  = nullptr;
            selected_tree = nullptr;
        }

        return;
    }

    // CREATE LINK
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (selected_pin != nullptr && hovered_pin != nullptr) {
            selected_tree->links.emplace_back(selected_pin, hovered_pin);
        }
        printf("Selected: %d, hovered %d, selected_tree %d\n",
               selected_pin != nullptr,
               hovered_pin != nullptr,
               selected_tree != nullptr);
        selected_pin  = nullptr;
        hovered_pin   = nullptr;
        selected_tree = nullptr;

        selected_node       = nullptr;
        rectangle_select    = false;
        rectangle_selection = ImRect();
        return;
    }

    // PENDING LINK
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        if (selected_pin != nullptr) {
            auto color = _colors[selected_pin->type()];
            draw_bezier(draw_list,
                        with_scroll(selected_pin->position()),
                        ImGui::GetMousePos(),
                        color,
                        bezier_segments,
                        tickness);
            return;
        }

        if (selected_node == nullptr) {
            if (!rectangle_select) {
                rectangle_select = true;
                for (auto& item: selected) {
                    item.first->selected() = false;
                }
                selected.clear();
                rectangle_start = ImGui::GetMousePos();
            }

            // ImVec2 mp = ImGui::GetMousePos();
            // ImVec2 mn = ImVec2(std::min(rectangle_start.x, mp.x),  std::min(rectangle_start.y,
            // mp.y)); ImVec2 mx = ImVec2(std::max(rectangle_start.x, mp.x),
            // std::max(rectangle_start.y, mp.y)); rectangle_selection = ImRect(mn, mx);

            rectangle_selection = ImRect(rectangle_start, rectangle_start);
            rectangle_selection.Add(ImGui::GetMousePos());

            draw_list->AddRectFilled(
                rectangle_selection.GetTR(), rectangle_selection.GetBL(), rectangle_color);
            return;
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

bool draw_bezier(ImDrawList* draw_list,
                 ImVec2      p1,
                 ImVec2      p2,
                 ImU32       color,
                 int         segments,
                 float       tickness,
                 float       eps) {
    if (p1.x > p2.x) {
        std::swap(p1, p2);
    }

    auto sgn = [](float v) -> int { return int(v > 0); };

    auto offset   = ImVec2((p2.x - p1.x + 1) / 2.f, 0);
    auto y_offset = sgn(p2.y - p1.y) * (p2.y - p1.y) / 2.f;

    if (p2.y - p1.y > p2.x - p1.x) {
        offset = ImVec2(0, y_offset);
    }

    ImVec2 P0 = p1;
    ImVec2 P1 = p1 + offset;
    ImVec2 P2 = p2 - offset;
    ImVec2 P3 = p2;

    const ImProjectResult result =
        ImProjectOnCubicBezier(ImGui::GetMousePos(), P0, P1, P2, P3, segments);

    bool hovered = result.Distance <= tickness + eps;

    draw_list->AddBezierCubic(P0, P1, P2, P3, color, tickness + float(hovered), segments);

    return hovered;
}

void GraphEditor::draw(Link* link, ImVec2 offset) {
    auto color = _colors[link->from->type()];

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    bool        hovered   = draw_bezier(draw_list,
                               with_scroll(link->from->position()),
                               with_scroll(link->to->position()),
                               color,
                               bezier_segments,
                               tickness);

    if (hovered) {
        hovered_link  = link;
        selected_tree = current_tree;
    }
}

ImVec2 GraphEditor::estimate_size(GraphNodeBase* node, ImVec2 font_size) {
    
    Array<GraphNodePinBase*> inputs;
    Array<GraphNodePinBase*> outputs;
    node->get_pins_by_direction(inputs, outputs);

    ImRect size;
    float  height = 0;
    for (int i = 0; i < inputs.size(); i++) {
        if (inputs[i]->name().empty())
            continue;

        ImVec2 v = font_size * ImVec2(inputs[i]->name().size(), 1);

        height += v.y;
        size.Add(v + ImVec2(pin_radius * 4, height));
    }

    ImVec2 v = font_size * ImVec2(node->name().size(), 1);
    size.Add(v + ImVec2(pin_radius * 4, height));

    ImVec2 col1 = size.GetSize();

    height = 0;
    for (int i = 0; i < outputs.size(); i++) {
        if (outputs[i]->name().empty())
            continue;

        ImVec2 v = font_size * ImVec2(outputs[i]->name().size(), 1);
        height += v.y;
        size.Add(v + ImVec2(col1.x, height) + ImVec2(pin_radius * 2, 0));
    }

    return size.GetSize();
}

void GraphEditor::draw(GraphNodeBase* node, ImVec2 offset) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImGuiIO&    io        = ImGui::GetIO();

    ImGui::PushID(int(node->id));

    ImVec2       node_rect_min = offset + node->position();  // Parent offset
    static float value;
    bool         old_any_active = ImGui::IsAnyItemActive();

    // Content
    draw_list->ChannelsSetCurrent(2);
    // ImGui::SetCursorScreenPos(node_rect_min + ImVec2(1, 1) * 0* node_padding);

    
    ImGui::BeginGroup();

    // Argument Column
    ImGui::BeginGroup();
    ImVec2 txt         = ImGui::CalcTextSize("T");
    float  line_height = node->input_position().y;
    float  line_width  = node->input_position().x;
    ImVec2 start       = node->position() + ImVec2(1, 1) * pin_radius + ImVec2(1, 1) * node_padding;
    float  new_width   = pin_radius / 2;
    float  new_height  = std::max(pin_radius * 2, txt.y);

    ImGui::SetCursorPos(start);
    _size = ImRect();

    Array<GraphNodePinBase*> inputs;
    Array<GraphNodePinBase*> outputs;
    node->get_pins_by_direction(inputs, outputs);
    GraphNodePinBase* exec_in = nullptr;
    GraphNodePinBase* exec_out = nullptr;

    // Inputs
    for (int slot_idx = 0; slot_idx < inputs.size(); slot_idx++) {
        GraphNodePinBase* pin = inputs[slot_idx];

        pin->position() = start;

        const char* name = pin->name().c_str();
        if (pin->kind() == PinKind::Flow) {
            name = node->name().c_str();
            exec_in = pin;
        }
        ImVec2 v = ImGui::CalcTextSize(name);

        draw(pin, with_scroll(pin->position()));
        ImGui::SetCursorPos(
            with_scroll(pin->position() + ImVec2(pin_radius / 2 + pin_label_margin, -(pin_radius + v.y / 4))));

        ImGui::Text("%s", name);
        ImVec2 s(0, 0);
        if (pin->type() == PinType::Float && !pin->connected()) {
            ImGui::SameLine();
            ImGui::PushItemWidth(txt.x * 5);
            // ImGui::InputFloat("", pin.as_float(), 0, 0, "%.f");
            ImGui::PopItemWidth();

            s = ImGui::GetItemRectSize();
        }

        new_height = std::max(new_height, v.y);
        new_width  = std::max(new_width, v.x + s.x);
        start.y += line_height;
    }
    node->input_position().y = new_height;
    node->input_position().x = new_width + pin_radius;

    ImGui::EndGroup();
    ImVec2 s = ImGui::GetItemRectSize();

    //
    if (exec_in == nullptr) {
        const char* name = node->name().c_str();

        ImVec2 sz = ImGui::CalcTextSize(name) + ImVec2(txt.x, 0);
        // ImVec2 sz(txt.x * (node->name.size() + 2), txt.y);

        // ImVec2 center = ImVec2(
        //     node->layout.input.x,
        //     (node->size.y - sz.y )/ 2.f
        // );

        ImVec2 center = node->size() / 2 + ImVec2(-sz.x, -sz.y - node_padding);
        ImGui::SetCursorPos(with_scroll(node->position() + center));
        ImGui::Text("%s", name);

        node->input_position().x = new_width + sz.x + 2 * pin_radius;
    }

    // Ouputs
    ImGui::SameLine();
    ImGui::BeginGroup();
    // Output Column
    // ImGui::SetCursorScreenPos(node_rect_min + ImVec2(node->layout.input.x, 0));

    ImGui::SetCursorPos(node->position() + ImVec2(1, 0) * pin_radius + ImVec2(node->input_position().x, 0) +
                        ImVec2(0, 1) * node_padding);
    start = ImGui::GetCursorPos();

    line_width = node->output_position().x;
    new_width  = pin_radius;
    new_height = pin_radius * 2;

    for (int slot_idx = 0; slot_idx < outputs.size(); slot_idx++) {
        GraphNodePinBase* pin         = outputs[slot_idx];
        pin->position()          = start + ImVec2(pin_radius / 2, 0);
        const char* name = pin->name().c_str();

        ImVec2 v = ImGui::CalcTextSize(name);
        if (pin->name().empty())
            v.y = 0;

        new_width  = std::max(new_width, v.x + pin_radius);
        new_height = std::max(new_height, v.y);
        float s    = line_width - v.x - pin_radius / 2;

        ImGui::SetCursorPos(with_scroll(start + ImVec2(s, -v.y / 4)));
        ImGui::Text("%s", name);

        pin->position() = start + ImVec2(line_width + pin_label_margin, 0) + ImVec2(1, 1) * pin_radius +
                  ImVec2(0, 0);
        draw(pin, with_scroll(pin->position()));

        start.y += line_height;
    }
    node->output_position().x = new_width + pin_label_margin;
    node->output_position().y = new_height;
    ImGui::EndGroup();
    ImGui::EndGroup();
    // ---

    node->size()           = (_size.GetSize() - node->position()) + ImVec2(1, 1) * node_padding;
    ImVec2 node_rect_max = node_rect_min + node->size();

    // Background
    draw_list->ChannelsSetCurrent(1);
    // ImGui::SetCursorScreenPos(node_rect_min);

    ImGui::SetCursorPos(with_scroll(node->position()));
    ImGui::InvisibleButton("node", node->size());
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

    if (node_moving_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {

        if (!has_selection()) {
            node->position() = node->position() + io.MouseDelta;
        } else {
            for (auto& item: selected) {
                item.first->position() = item.first->position() + io.MouseDelta;
            }
        }
    }

    // ----

    draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
    draw_list->AddRect(node_rect_min,
                       node_rect_max,
                       node->selected() ? node_selected_color : node_outline_color,
                       4.0f,
                       0,
                       node->selected() ? 4.0f : 1.0f);
    check_selected(node);

    ImGui::PopID();
}

void GraphEditor::draw(GraphNodePinBase* pin, ImVec2 center) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSetCurrent(2);

    ImGui::PushID(int(pin->id));

    ImVec2 radius  = ImVec2(1, 1) * pin_radius;
    bool   hovered = false;
    bool   held    = false;
    auto   flags   = ImGuiButtonFlags_NoHoldingActiveId;

    ImVec2 pos  = center - radius - get_offset();
    ImVec2 size = radius * 2;
    ImRect bb(pos, pos + size);

    ImGui::SetCursorPos(with_scroll(pos));
    ImGui::ItemSize(size, 10);
    ImGui::ItemAdd(bb, int(pin->id));
    ImGui::ButtonBehavior(                         //
        ImRect(center - radius, center + radius),  //
        int(pin->id),                              //
        &hovered,                                  //
        &held,                                     //
        flags                                      //
    );

    // ButtonBehavior hovered does not work well
    hovered = bb.Contains(ImGui::GetMousePos());

    // ImGui::SetCursorPos(with_scroll(pos));
    // ImGui::Button("", size);

    _size.Add(pos);
    _size.Add(pos + size);

    if (held) {
        selected_pin  = pin;
        selected_tree = current_tree;
    }

    if (hovered) {
        hovered_pin = pin;
    }

    ImU32 color = _colors[pin->type()];

    auto style = PinStyle{
        //
        pin->kind(),       //
        pin->connected(),  //
        color,           //
        color            //
    };

    draw(style, center - radius, radius * 2.0);
    ImGui::PopID();
}

void GraphEditor::draw(PinStyle& style, ImVec2 pos, ImVec2 size) {

    switch (style.kind) {
    case PinKind::Flow: return draw_flow(style, pos, size);
    case PinKind::Circle: return draw_circle(style, pos, size);
    case PinKind::Square: return draw_square(style, pos, size);
    case PinKind::Grid: return draw_grid(style, pos, size);
    case PinKind::RoundSquare: return draw_round_square(style, pos, size);
    case PinKind::Diamond: return draw_diamond(style, pos, size);
    }

    return draw_circle(style, pos, size);
}

void GraphEditor::draw_flow(PinStyle& style, ImVec2 pos, ImVec2 size) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto       rect           = ImRect(pos, pos + size);
    auto       rect_x         = rect.Min.x;
    auto       rect_y         = rect.Min.y;
    auto       rect_w         = rect.Max.x - rect.Min.x;
    auto       rect_h         = rect.Max.y - rect.Min.y;
    auto       rect_center_x  = (rect.Min.x + rect.Max.x) * 0.5f;
    auto       rect_center_y  = (rect.Min.y + rect.Max.y) * 0.5f;
    auto       rect_center    = ImVec2(rect_center_x, rect_center_y);
    const auto outline_scale  = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale);  // for full circle

    const auto origin_scale = rect_w / 24.0f;
    const auto offset_x     = 1.0f * origin_scale;
    const auto offset_y     = 0.0f * origin_scale;
    const auto margin       = (style.filled ? 2.0f : 2.0f) * origin_scale;
    const auto rounding     = 0.1f * origin_scale;
    const auto tip_round    = 0.7f;  // percentage of triangle edge (for tip)
    // const auto edge_round = 0.7f; // percentage of triangle edge (for corner)
    const auto canvas   = ImRect(rect.Min.x + margin + offset_x,
                               rect.Min.y + margin + offset_y,
                               rect.Max.x - margin + offset_x,
                               rect.Max.y - margin + offset_y);
    const auto canvas_x = canvas.Min.x;
    const auto canvas_y = canvas.Min.y;
    const auto canvas_w = canvas.Max.x - canvas.Min.x;
    const auto canvas_h = canvas.Max.y - canvas.Min.y;

    const auto left     = canvas_x + canvas_w * 0.5f * 0.3f;
    const auto right    = canvas_x + canvas_w - canvas_w * 0.5f * 0.3f;
    const auto top      = canvas_y + canvas_h * 0.5f * 0.2f;
    const auto bottom   = canvas_y + canvas_h - canvas_h * 0.5f * 0.2f;
    const auto center_y = (top + bottom) * 0.5f;
    // const auto angle = AX_PI * 0.5f * 0.5f * 0.5f;

    const auto tip_top    = ImVec2(canvas_x + canvas_w * 0.5f, top);
    const auto tip_right  = ImVec2(right, center_y);
    const auto tip_bottom = ImVec2(canvas_x + canvas_w * 0.5f, bottom);

    draw_list->PathLineTo(ImVec2(left, top) + ImVec2(0, rounding));
    draw_list->PathBezierCubicCurveTo(
        ImVec2(left, top), ImVec2(left, top), ImVec2(left, top) + ImVec2(rounding, 0));
    draw_list->PathLineTo(tip_top);
    draw_list->PathLineTo(tip_top + (tip_right - tip_top) * tip_round);
    draw_list->PathBezierCubicCurveTo(
        tip_right, tip_right, tip_bottom + (tip_right - tip_bottom) * tip_round);
    draw_list->PathLineTo(tip_bottom);
    draw_list->PathLineTo(ImVec2(left, bottom) + ImVec2(rounding, 0));
    draw_list->PathBezierCubicCurveTo(
        ImVec2(left, bottom), ImVec2(left, bottom), ImVec2(left, bottom) - ImVec2(0, rounding));

    if (style.filled) {
        if (style.fill & 0xFF000000) {
            draw_list->AddConvexPolyFilled(
                draw_list->_Path.Data, draw_list->_Path.Size, style.fill);
        }
    }
    draw_list->PathStroke(style.color, true, 2.0f * outline_scale);
}

void GraphEditor::draw_circle(PinStyle& style, ImVec2 pos, ImVec2 size) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto       rect           = ImRect(pos, pos + size);
    auto       rect_x         = rect.Min.x;
    auto       rect_y         = rect.Min.y;
    auto       rect_w         = rect.Max.x - rect.Min.x;
    auto       rect_h         = rect.Max.y - rect.Min.y;
    auto       rect_center_x  = (rect.Min.x + rect.Max.x) * 0.5f;
    auto       rect_center_y  = (rect.Min.y + rect.Max.y) * 0.5f;
    auto       rect_center    = ImVec2(rect_center_x, rect_center_y);
    const auto outline_scale  = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale);  // for full circle

    auto triangleStart = rect_center_x + 0.32f * rect_w;
    auto rect_offset   = -static_cast<int>(rect_w * 0.25f * 0.25f);
    rect.Min.x += rect_offset;
    rect.Max.x += rect_offset;
    rect_x += rect_offset;
    rect_center_x += rect_offset * 0.5f;
    rect_center.x += rect_offset * 0.5f;

    const auto c = rect_center;
    if (style.filled) {
        const auto r = 0.5f * rect_w / 2.0f - 0.5f;

        if (style.fill & 0xFF000000) {
            draw_list->AddCircleFilled(c, r, style.fill, 12 + extra_segments);
        }

        draw_list->AddCircle(c, r, style.color, 12 + extra_segments, 2.0f * outline_scale);
    } else {
        draw_list->AddCircle(c, 0.5f * rect_w / 2.0f, style.color, 12 + extra_segments);
    }

    const auto triangleTip = triangleStart + rect_w * (0.45f - 0.32f);

    draw_list->AddTriangleFilled(ImVec2(ceilf(triangleTip), rect_y + rect_h * 0.5f),
                                 ImVec2(triangleStart, rect_center_y + 0.15f * rect_h),
                                 ImVec2(triangleStart, rect_center_y - 0.15f * rect_h),
                                 style.color);
}

void GraphEditor::draw_square(PinStyle& style, ImVec2 pos, ImVec2 size) {

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto       rect           = ImRect(pos, pos + size);
    auto       rect_x         = rect.Min.x;
    auto       rect_y         = rect.Min.y;
    auto       rect_w         = rect.Max.x - rect.Min.x;
    auto       rect_h         = rect.Max.y - rect.Min.y;
    auto       rect_center_x  = (rect.Min.x + rect.Max.x) * 0.5f;
    auto       rect_center_y  = (rect.Min.y + rect.Max.y) * 0.5f;
    auto       rect_center    = ImVec2(rect_center_x, rect_center_y);
    const auto outline_scale  = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale);  // for full circle

    auto triangleStart = rect_center_x + 0.32f * rect_w;
    auto rect_offset   = -static_cast<int>(rect_w * 0.25f * 0.25f);
    rect.Min.x += rect_offset;
    rect.Max.x += rect_offset;
    rect_x += rect_offset;
    rect_center_x += rect_offset * 0.5f;
    rect_center.x += rect_offset * 0.5f;

    if (style.filled) {
        const auto r  = 0.5f * rect_w / 2.0f;
        const auto p0 = rect_center - ImVec2(r, r);
        const auto p1 = rect_center + ImVec2(r, r);

        draw_list->AddRectFilled(p0, p1, style.color, 0, ImDrawFlags_RoundCornersAll);
    } else {
        const auto r  = 0.5f * rect_w / 2.0f - 0.5f;
        const auto p0 = rect_center - ImVec2(r, r);
        const auto p1 = rect_center + ImVec2(r, r);

        if (style.fill & 0xFF000000) {
            draw_list->AddRectFilled(p0, p1, style.fill, 0, ImDrawFlags_RoundCornersAll);
        }

        draw_list->AddRect(
            p0, p1, style.color, 0, ImDrawFlags_RoundCornersAll, 2.0f * outline_scale);
    }
}

void GraphEditor::draw_grid(PinStyle& style, ImVec2 pos, ImVec2 size) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto       rect           = ImRect(pos, pos + size);
    auto       rect_x         = rect.Min.x;
    auto       rect_y         = rect.Min.y;
    auto       rect_w         = rect.Max.x - rect.Min.x;
    auto       rect_h         = rect.Max.y - rect.Min.y;
    auto       rect_center_x  = (rect.Min.x + rect.Max.x) * 0.5f;
    auto       rect_center_y  = (rect.Min.y + rect.Max.y) * 0.5f;
    auto       rect_center    = ImVec2(rect_center_x, rect_center_y);
    const auto outline_scale  = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale);  // for full circle

    auto triangleStart = rect_center_x + 0.32f * rect_w;

    auto rect_offset = -static_cast<int>(rect_w * 0.25f * 0.25f);

    rect.Min.x += rect_offset;
    rect.Max.x += rect_offset;
    rect_x += rect_offset;
    rect_center_x += rect_offset * 0.5f;
    rect_center.x += rect_offset * 0.5f;

    const auto r = 0.5f * rect_w / 2.0f;
    const auto w = ceilf(r / 3.0f);

    const auto baseTl = ImVec2(floorf(rect_center_x - w * 2.5f), floorf(rect_center_y - w * 2.5f));
    const auto baseBr = ImVec2(floorf(baseTl.x + w), floorf(baseTl.y + w));

    auto tl = baseTl;
    auto br = baseBr;
    for (int i = 0; i < 3; ++i) {
        tl.x = baseTl.x;
        br.x = baseBr.x;
        draw_list->AddRectFilled(tl, br, style.color);
        tl.x += w * 2;
        br.x += w * 2;

        if (i != 1 || style.filled)
            draw_list->AddRectFilled(tl, br, style.color);

        tl.x += w * 2;
        br.x += w * 2;
        draw_list->AddRectFilled(tl, br, style.color);

        tl.y += w * 2;
        br.y += w * 2;
    }

    // ---
    triangleStart          = br.x + w + 1.0f / 24.0f * rect_w;
    const auto triangleTip = triangleStart + rect_w * (0.45f - 0.32f);
    draw_list->AddTriangleFilled(ImVec2(ceilf(triangleTip), rect_y + rect_h * 0.5f),
                                 ImVec2(triangleStart, rect_center_y + 0.15f * rect_h),
                                 ImVec2(triangleStart, rect_center_y - 0.15f * rect_h),
                                 style.color);
}

void GraphEditor::draw_round_square(PinStyle& style, ImVec2 pos, ImVec2 size) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto       rect           = ImRect(pos, pos + size);
    auto       rect_x         = rect.Min.x;
    auto       rect_y         = rect.Min.y;
    auto       rect_w         = rect.Max.x - rect.Min.x;
    auto       rect_h         = rect.Max.y - rect.Min.y;
    auto       rect_center_x  = (rect.Min.x + rect.Max.x) * 0.5f;
    auto       rect_center_y  = (rect.Min.y + rect.Max.y) * 0.5f;
    auto       rect_center    = ImVec2(rect_center_x, rect_center_y);
    const auto outline_scale  = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale);  // for full circle

    auto triangleStart = rect_center_x + 0.32f * rect_w;
    auto rect_offset   = -static_cast<int>(rect_w * 0.25f * 0.25f);
    rect.Min.x += rect_offset;
    rect.Max.x += rect_offset;
    rect_x += rect_offset;
    rect_center_x += rect_offset * 0.5f;
    rect_center.x += rect_offset * 0.5f;

    if (style.filled) {
        const auto r  = 0.5f * rect_w / 2.0f;
        const auto cr = r * 0.5f;
        const auto p0 = rect_center - ImVec2(r, r);
        const auto p1 = rect_center + ImVec2(r, r);

        draw_list->AddRectFilled(p0, p1, style.color, cr, ImDrawFlags_RoundCornersAll);
    } else {
        const auto r  = 0.5f * rect_w / 2.0f - 0.5f;
        const auto cr = r * 0.5f;
        const auto p0 = rect_center - ImVec2(r, r);
        const auto p1 = rect_center + ImVec2(r, r);

        if (style.fill & 0xFF000000) {
            draw_list->AddRectFilled(p0, p1, style.fill, cr, ImDrawFlags_RoundCornersAll);
        }

        draw_list->AddRect(
            p0, p1, style.color, cr, ImDrawFlags_RoundCornersAll, 2.0f * outline_scale);
    }
}
void GraphEditor::draw_diamond(PinStyle& style, ImVec2 pos, ImVec2 size) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto       rect           = ImRect(pos, pos + size);
    auto       rect_x         = rect.Min.x;
    auto       rect_y         = rect.Min.y;
    auto       rect_w         = rect.Max.x - rect.Min.x;
    auto       rect_h         = rect.Max.y - rect.Min.y;
    auto       rect_center_x  = (rect.Min.x + rect.Max.x) * 0.5f;
    auto       rect_center_y  = (rect.Min.y + rect.Max.y) * 0.5f;
    auto       rect_center    = ImVec2(rect_center_x, rect_center_y);
    const auto outline_scale  = rect_w / 24.0f;
    const auto extra_segments = static_cast<int>(2 * outline_scale);  // for full circle

    auto triangleStart = rect_center_x + 0.32f * rect_w;
    auto rect_offset   = -static_cast<int>(rect_w * 0.25f * 0.25f);
    rect.Min.x += rect_offset;
    rect.Max.x += rect_offset;
    rect_x += rect_offset;
    rect_center_x += rect_offset * 0.5f;
    rect_center.x += rect_offset * 0.5f;

    if (style.filled) {
        const auto r = 0.607f * rect_w / 2.0f;
        const auto c = rect_center;

        draw_list->PathLineTo(c + ImVec2(0, -r));
        draw_list->PathLineTo(c + ImVec2(r, 0));
        draw_list->PathLineTo(c + ImVec2(0, r));
        draw_list->PathLineTo(c + ImVec2(-r, 0));
        draw_list->PathFillConvex(style.color);
    } else {
        const auto r = 0.607f * rect_w / 2.0f - 0.5f;
        const auto c = rect_center;

        draw_list->PathLineTo(c + ImVec2(0, -r));
        draw_list->PathLineTo(c + ImVec2(r, 0));
        draw_list->PathLineTo(c + ImVec2(0, r));
        draw_list->PathLineTo(c + ImVec2(-r, 0));

        if (style.fill & 0xFF000000)
            draw_list->AddConvexPolyFilled(
                draw_list->_Path.Data, draw_list->_Path.Size, style.fill);

        draw_list->PathStroke(style.color, true, 2.0f * outline_scale);
    }
}

void GraphEditor::draw_triangle(PinStyle& style, ImVec2 pos, ImVec2 size) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto       rect          = ImRect(pos, pos + size);
    auto       rect_x        = rect.Min.x;
    auto       rect_y        = rect.Min.y;
    auto       rect_w        = rect.Max.x - rect.Min.x;
    auto       rect_h        = rect.Max.y - rect.Min.y;
    auto       rect_center_x = (rect.Min.x + rect.Max.x) * 0.5f;
    auto       rect_center_y = (rect.Min.y + rect.Max.y) * 0.5f;
    auto       rect_center   = ImVec2(rect_center_x, rect_center_y);
    auto       triangleStart = rect_center_x + 0.32f * rect_w;
    const auto triangleTip   = triangleStart + rect_w * (0.45f - 0.32f);

    draw_list->AddTriangleFilled(ImVec2(ceilf(triangleTip), rect_y + rect_h * 0.5f),
                                 ImVec2(triangleStart, rect_center_y + 0.15f * rect_h),
                                 ImVec2(triangleStart, rect_center_y - 0.15f * rect_h),
                                 style.color);
}

}