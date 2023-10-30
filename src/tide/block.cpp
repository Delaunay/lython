#include "tide/block.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

void StmtBlock::draw() {}

void FlowBlock::draw() {
    ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
    ImGui::Begin("FlowBlock");
    auto offset = ImGui::GetWindowPos();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    auto color = IM_COL32(255, 255, 120, 255);
    float width = 15;
    float height = 10;
    ImGui::GetStyle().AntiAliasedFill = false;

    ImVec2 u = ImVec2(8, 8);
    ImVec2 o = ImVec2(32, 32) + offset;
    ImVec2 p1[5] = {
        ImVec2(0, 0) * u + o,
        ImVec2(1, 0) * u + o,
        ImVec2(2, 1) * u + o,
        ImVec2(3, 3) * u + o,
        ImVec2(0, 3) * u + o
    };
    ImVec2 p2[5] = {
        ImVec2(2, 1) * u + o,
        ImVec2(4, 1) * u + o,
        ImVec2(6, 4) * u + o,
        ImVec2(4, 4) * u + o,
        ImVec2(3, 3) * u + o
    };
    ImVec2 p3[4] = {
        ImVec2(4, 1) * u + o,
        ImVec2(5, 0) * u + o,
        ImVec2(7, 3) * u + o,
        ImVec2(6, 4) * u + o,
    };
    ImVec2 p4[4] = {
        ImVec2(5, 0) * u + o,
        ImVec2(width, 0) * u + o,
        ImVec2(width, 3) * u + o,
        ImVec2(7, 3) * u + o,
    };
    draw_list->AddConvexPolyFilled(p1, 5, color);
    draw_list->AddConvexPolyFilled(p2, 5, color);
    draw_list->AddConvexPolyFilled(p3, 4, color);
    draw_list->AddConvexPolyFilled(p4, 4, color);
    // 

    ImVec2 p5[4] = {
        ImVec2(0, 3) * u + o,
        ImVec2(2, 3) * u + o,
        ImVec2(2, height) * u + o,
        ImVec2(0, height) * u + o,
    };
    draw_list->AddConvexPolyFilled(p5, 4, color);

    ImVec2 p6[4] = {
        ImVec2(0, height) * u + o,
        ImVec2(3, height) * u + o,
        ImVec2(1, height + 3) * u + o,
        ImVec2(0, height + 3) * u + o,
    };
    draw_list->AddConvexPolyFilled(p6, 4, color);

    ImVec2 p7[4] = {
        ImVec2(3, height) * u + o,
        ImVec2(4, height + 1) * u + o,
        ImVec2(2, height + 4) * u + o,
        ImVec2(1, height + 3) * u + o,
    };
    draw_list->AddConvexPolyFilled(p7, 4, color);

    ImVec2 p8[5] = {
        ImVec2(4, height + 1) * u + o,
        ImVec2(6, height + 1) * u + o,
        ImVec2(5, height + 3) * u + o,
        ImVec2(4, height + 4) * u + o,
        ImVec2(2, height + 4) * u + o,
    };
    draw_list->AddConvexPolyFilled(p8, 5, color);

    ImVec2 p9[5] = {
        ImVec2(6, height + 1) * u + o,
        ImVec2(7, height) * u + o,
        ImVec2(width, height) * u + o,
        ImVec2(width, height + 3) * u + o,
        ImVec2(5, height + 3) * u + o,
    };
    draw_list->AddConvexPolyFilled(p9, 5, color);
    ImGui::End();
}

void ExprBlock::draw() {}

void EventBlock::draw() {}
