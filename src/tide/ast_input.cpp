#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

#include "ast/ops.h"

#include "tide/ast_input.h"
#include "tide/ast_render.h"

namespace lython {
// InputState& InputState::state() {
//     static InputState state;
//     return state;
//  }

template <typename T>
T clamp(T a, T mn, T mx) {
    if (a > mx)
        return mx;
    if (a < mn)
        return mn;
    return a;
};

template <typename Action>
void input_action_pressed_held(float dt, float& heldtime, float speed, ImGuiKey Key, Action fun) {
    ImGuiKeyData* key_data = ImGui::GetKeyData(Key);

    if (!key_data->Down && key_data->DownDurationPrev >= 0) {
        fun();
        heldtime = 0;
    } else if (key_data->Down && key_data->DownDuration > 0) {
        heldtime += dt;
        if (heldtime > speed) {
            fun();
            heldtime = 0;
        }
    }
}

void clicked(ImGuiMouseButton button) {
    ImGui::GetCursorPos();
    ImGui::IsMouseDoubleClicked(button);
}

ASTEditor::ASTEditor() { renderer.style = &style; }

void ASTEditor::input(float dt) {
    if (renderer._redraw) {
        renderer.drawings.clear();
        renderer.groups.clear();
        renderer.edit_order.clear();
        for(auto entity: renderer.entities) {
            entity.destruct();
        }
        renderer.entities.clear();
        renderer.cursor = renderer.start;
        // renderer.run(module);
    };

    ImGuiIO& io = ImGui::GetIO();
    int      n  = int(renderer.edit_order.size());

    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        ImVec2 pos = ImGui::GetCursorPos();
    }

    auto get_insert_cursor = [this](Group* group) -> int {
        if (group == nullptr) {
            return -1;
        }
        ImVec2 char_size = this->style.font->CalcTextSizeA(
            this->style.font_size, FLT_MAX, 0.0f, " ");
        float char_width = char_size.x;

        float insert_pos = 0;
        if (group->drawing) {

            // text being modified
            ImRect position = group->drawing->rectangle;
            String const& text = group->drawing->string;

            ImVec2 pos = this->blinky_px - position.Min;
            insert_pos = pos.x / char_width;
        }
        return insert_pos;
    };

    auto move_blinky = [this](float dir){
        ImVec2 char_size = this->style.font->CalcTextSizeA(
            this->style.font_size, FLT_MAX, 0.0f, " ");
        float char_width = char_size.x;

        blinky += ImVec2(1, 0) * dir;
        blinky_px += ImVec2(char_width, 0) * dir;
    };


    input_action_pressed_held(dt, pop_time, pop_speed, ImGuiKey_RightArrow, [this, n]() {
        this->index = clamp(this->index + 1, 0, n - 1);
        // blinky.x += 1;
    });
    input_action_pressed_held(dt, pop_time, pop_speed, ImGuiKey_LeftArrow, [this, n]() {
        this->index = clamp(this->index - 1, 0, n - 1);
        // blinky.x -= 1;
    });
    input_action_pressed_held(dt, pop_time, pop_speed, ImGuiKey_Tab, [this, n]() {
        this->index = clamp(this->index + 1, 0, n - 1);
    });
    input_action_pressed_held(dt, pop_time, pop_speed, ImGuiKey_Backspace, [&, this]() {
        // this->input_buffer.pop_back();
        if (this->index >= 0 && !renderer.edit_order.empty()) {
            int idx = renderer.edit_order[this->index];
            Group* group = renderer.groups[idx].get_mut<Group>();

            // do samething for input & backspace
            if (group->backspace) {
                int insert_pos = get_insert_cursor(group);
                bool success = group->backspace(insert_pos - 1);

                if (success) {
                    move_blinky(-1);
                }
            }
        }
    });

    if (this->index >= 0 && !renderer.edit_order.empty()) {
        int idx = renderer.edit_order[this->index];
        Group* group = renderer.groups[idx].get_mut<Group>();

        if (io.InputQueueCharacters.Size > 0) {
            // FIXME we need an undo
            // we should give the entire queue to input and input should generate a undo entry
            int insert_pos = get_insert_cursor(group);

            for (int n = 0; n < io.InputQueueCharacters.Size; n++) {
                unsigned int c = (unsigned int)io.InputQueueCharacters[n];
                // input_buffer.push_back(c);
                if (group->input) {
                    bool success = group->input(int(insert_pos), c);
                    if (success) {
                        move_blinky(1);
                    }
                }
            }
            
            io.InputQueueCharacters.resize(0);
        }
    }
}

void ASTEditor::update_trie() {
    Array<String> statements = {
        "def",          // create a function
        "while",        // create a while statement
        "for",
        "match",
        "class",
        "return",
        "async",
        "del",
        "if",
        "with",
        "raise",
        "try",
        "assert",
        "import",
        "from",
        "global",
        "nonlocal",
        "pass",
        "break",
        "continue",
        "expr",        // <= Insert ExprStmt to hold an expression
    };

    //suggest_trie.insert(SuggestTrieContext::Statement);
    //SuggestTrie* stmt = suggest_trie.matching(SuggestTrieContext::Statement);

    SuggestTrie* stmt  = &suggest_trie;
    for(String const& str: statements) {
        stmt->insert(str);
    }
}

void ASTEditor::suggest() {
    update_trie();
    suggestions = suggest_trie.complete(input_buffer);
}

void ASTEditor::test() {
    ImVec2 pos = ImGui::GetMousePos();
    ImVec2 char_size = style.font->CalcTextSizeA(
        style.font_size, FLT_MAX, 0.0f, " ");


    // int count = 0;
    // for(Drawing& draw: renderer.drawings) {
    //     if (draw.rectangle.Contains(pos)) {
    //         count += 1;

    //         ImDrawList* drawlist = ImGui::GetWindowDrawList();
    //         drawlist->AddRect(draw.rectangle.Min, draw.rectangle.Max, ImColor(255, 255, 255), 0,
    //         0, 1);

    //     }
    // }

    // static int prev = -1;
    // if (prev != count) {
    //     std::cout << "Found: " << count << std::endl;
    //     prev = count;
    // }

    auto show = [](ImRect x) -> std::ostream& {
        return std::cout << x.Min.x << " " << x.Min.y << " " << x.Max.x << " " << x.Max.y;
    };

    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    // for (Group* group: renderer.groups) 
    {

        // drawlist->AddRect(group.rectangle.Min, group.rectangle.Max, ImColor(255, 255, 255), 0, 0,
        // 1);
    }

    Group* body = nullptr;
    Group* expr = nullptr;
    int i = 0;
    for (auto entity: renderer.groups) {
        Group* group = entity.get_mut<Group>();

        if (group->rectangle.Contains(pos)) {
            if (group->body != nullptr) {
                body = group;
                break;
            }

            if (expr == nullptr) {
                expr = group;

                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    index = expr->edit_id;
                    // get the text here
                
                    // I have the group but not the drawing that holds
                    // the string to deduce the cursor
                    // but given the size of the box I might be able to deduce it
                    //ImVec2 offset = ImVec2(50, 20);
                    //string_cursor = pos.y / char_size.y;
                    //line = pos.x / char_size.x;
                }
            }
        }
        i += 1;
    }

    if (body) {
        drawlist->AddRect(body->rectangle.Min, body->rectangle.Max, ImColor(0, 255, 255), 0, 0, 1);
    }
    if (expr) {
        drawlist->AddRect(expr->rectangle.Min, expr->rectangle.Max, ImColor(0, 0, 255), 0, 0, 1);

        Node* node = expr->node;
        if (node) {
            String kind = str(node->kind);
            drawlist->AddText(ImVec2(500, 500), ImColor(155, 155, 155), kind.c_str());
        }
    }

    if (index >= 0 && !renderer.edit_order.empty()) {
        int idx = renderer.edit_order[index];
        Group* group = renderer.groups[idx].get_mut<Group>();
        
        if (group->drawing) {
            input_buffer = group->drawing->string;
            suggest();
        }
    }

}

void ASTEditor::draw(float dt) {
    ImVec2 offset = ImVec2(60, 20);
    ImVec2 pos    = ImGui::GetMousePos();
    ImVec2 size   = style.font->CalcTextSizeA(style.font_size, FLT_MAX, 0.0f, "    ");

    float line_height = (size.y + style.extra_line_space);
    int   line        = int((pos.y - 0.5 * line_height) / line_height);
    float line_px     = float(line + 1) * line_height - style.extra_line_space * 0.5f + offset.y;

    auto get_line = [&](float y) -> int {
        float char_height = (size.y + this->style.extra_line_space);
        return int(float((y - offset.y - 0.0 * char_height) / char_height));
    };

    auto get_col = [&](float x) -> int {
        float char_width = size.x / 4;
        return int((x - offset.x - 0.5 * char_width) / char_width);
    };

    ImDrawList* drawlist = ImGui::GetWindowDrawList();



    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            // I have the group but not the drawing that holds
            // the string to deduce the cursor
            // but given the size of the box I might be able to deduce it
            blinky.x = get_col(pos.x) + 1;
            blinky.y = get_line(pos.y) + 1;
        }

        float char_width = size.x / 4;
        blinky_px = ImVec2(
            (blinky.x + 0.0) * char_width + offset.x,
            (blinky.y - 1.0) * line_height - style.extra_line_space * 0.0f + offset.y
        );

        static float tt = 0;
        static bool on = true;
        tt += dt;

        if (tt > 0.5) {
            on = !on;
            tt = 0;
        }
        if (on) {
            drawlist->AddLine(
                blinky_px + ImVec2(0, style.extra_line_space * 0.5),
                blinky_px + ImVec2(0, size.y - style.extra_line_space * 0.5),
                ImColor(255, 255, 255),
                1.0f    
            );
        }

        drawlist->AddRect(
            offset,
            offset + ImVec2(1, 1) * 1000,
            ImColor(255, 255, 255)
        );
    }

// Draw current line
#if 0
        drawlist->AddRect(ImVec2(60, line_px), ImVec2(1024, line_px), ImColor(255, 255, 255), 0, 0, 1);
#endif

    // Draw line number
    for (int i = 0; i < 30; i++) {
        float py_s = line_height * i + offset.y + style.extra_line_space * 0.5f;
        float py_e = line_height * (i + 1) + offset.y - style.extra_line_space * 0.5f;

        StringStream ss;
        ss << i;
        String s = ss.str();

        // drawlist->AddRect(ImVec2(0, py_s), ImVec2(size.x, py_e), ImColor(255, 255, 255), 0, 0,
        // 1);
        drawlist->AddText(
            ImVec2(size.x - s.size() * size.x / 4, py_s), ImColor(255, 255, 255), s.c_str());
    }

    // Show suggestion tooltip
    // suggest();

    if (!suggestions.empty()) {
        ImVec2 margin(10, 10);
        ImVec2 cursor = ImVec2(pos.x, line_px);
        ImRect rect(cursor, cursor);

        for (int i = 0; i < suggestions.size(); i++) {
            ImVec2 size =
                style.font->CalcTextSizeA(style.font_size, FLT_MAX, 0.0f, suggestions[i].c_str());
            rect.Add(rect.GetBL() + size + ImVec2(0, style.extra_line_space));
        }

        rect.Add(rect.GetBR() + margin);

        drawlist->AddRectFilled(rect.Min, rect.Max, ImColor(0, 0, 0), 0, 0);

        cursor += margin / 2;
        for (int i = 0; i < suggestions.size(); i++) {
            drawlist->AddText(cursor, ImColor(255, 255, 255), suggestions[i].c_str());
            cursor += ImVec2(0, line_height);
        }
    }

    // Show sema error
    // if (sema != nullptr)
    {
        float col = 550;
        ImVec2 cursor = ImVec2(col, 0);

        for(auto& error: sema.errors) {
            const char* txt = error->what();
            drawlist->AddText(cursor, ImColor(255, 255, 255), txt);
            ImVec2 size =  style.font->CalcTextSizeA(style.font_size, FLT_MAX, 0.0f, txt);
            cursor = ImVec2(col, size.y + cursor.y);
        }
    }

    // draw current buffer
    float  current_line = float(line) * line_height - style.extra_line_space * 0.5f + offset.y;
    ImVec2 cursor       = ImVec2(60, current_line);
    // drawlist->AddText(cursor, ImColor(255, 255, 255), input_buffer.c_str());

    // Debug draw
    {
        static ImVec2 size;
        int           i = 0;
        for (int k: renderer.edit_order) {
            auto pos = ImVec2(ImGui::GetWindowWidth() - size.x, 10 + i * line_height);

            if (i == index) {
                drawlist->AddRectFilled(pos, pos + size, ImColor(0, 0, 0), 0, 0);
            }

            Group* group = renderer.groups[k].get_mut<Group>();
            ImGui::SetCursorPos(pos);
            ImGui::Text("Group %3d (%3d:%2d) -> (%3d:%2d)",
                        group->id,
                        get_line(group->rectangle.Min.y),
                        get_col(group->rectangle.Min.x),
                        get_line(group->rectangle.Max.y),
                        get_col(group->rectangle.Max.x));  // str(group->node).c_str());
            ImVec2 s = ImGui::GetItemRectSize();
            size     = ImVec2(std::max(s.x, size.x), std::max(s.y, size.y));
            i += 1;
        }
    }

    // ImGui::InputText();

    // auto clamp_get = [](Array<T> const& array, int idx) {
    //     return array[clamp(idx, 0, array.size() - 1)];
    // };

    //
    if (renderer.edit_order.size() > 0) {
        int idx = renderer.edit_order[clamp(index, 0, int(renderer.edit_order.size()) - 1)];
        if (idx >= 0) {
            Group* selected = renderer.groups[clamp(idx, 0, int(renderer.groups.size()) - 1)].get_mut<Group>();

            drawlist->AddRect(
                selected->rectangle.Min, selected->rectangle.Max, ImColor(255, 0, 0), 0, 0, 1);
        }
    }

    // Blinking cursor
    // drawlist->AddLine();
}

}  // namespace lython