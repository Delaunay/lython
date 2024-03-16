#pragma once

#include <iostream>

#include "dtypes.h"

#include "sema/sema.h"
#include "tide/ast_render.h"

namespace lython {

struct InputState {
    // String buffer;

    // int active_id;

    // void set(int id, const char* str) {
    //     active_id = id;
    //     int newsize = std::max(int(buffer.size()), int(strlen(str) * 1.25));
    //     buffer.reserve(newsize);
    //     memset(buffer.data(), '\0', newsize);
    //     buffer += str;
    //     std::cout << "bufer " << buffer << std::endl;
    // }

    // void add_character(int c) {
    //     buffer.push_back(c);
    // }

    // static InputState& state();
};

struct ASTEditor {
    ASTEditor();

    float pop_speed = 0.150;
    float pop_time = 0;

    ImVec2 blinky;
    ImVec2 blinky_px;
    
    void draw(float dt);
    void input(float dt);

    int index = 0;
    // int focused = ;
    String input_buffer;
    int    input_cursor = -1;

    void suggest();

    Array<String> suggestions;

    struct Node* focused_node;
    ASTRenderStyle style;
    ASTRender renderer;

    SemanticAnalyser sema = nullptr;

    void draw_lines_num();
    void draw_current_line();

    void test() ;
};

}  // namespace lython