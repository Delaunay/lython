
struct ASTRenderContext {
    //
    ImVec2          start    = ImVec2(10, 10);
    ASTRenderStyle* style    = nullptr;
    ImVec2          pos      = ImVec2(10, 10);
    float           maxcol   = 0;
    int             _indent  = 0;
    bool            _comment = false;
    unsigned int    _id      = 1;

    void inline_string(const char* str, ImColor color);

    ASTRenderContext& operator<<(special::Docstring const& str);
    ASTRenderContext& operator<<(special::Type const& str);
    ASTRenderContext& operator<<(special::Indent const& str);
    ASTRenderContext& operator<<(special::BeforeComment const& str);
    ASTRenderContext& operator<<(special::Newline const& str);
    ASTRenderContext& operator<<(special::Keyword const& str);
    ASTRenderContext& operator<<(const char* str);
    ASTRenderContext& operator<<(String const& str);

    template <typename T>
    ASTRenderContext& operator<<(T const& value) {
        StringStream ss;
        ss << value;
        return (*this) << ss.str();
    }

    GenericGuard type() {
        return GenericGuard(
            [this]() { ImGui::PushStyleColor(ImGuiCol_Text, this->style->type.Value); },
            []() { ImGui::PopStyleColor(); });
    }

    GenericGuard comment() {
        return GenericGuard(
            [this]() { ImGui::PushStyleColor(ImGuiCol_Text, this->style->comment.Value); },
            []() { ImGui::PopStyleColor(); });
    }

    GenericGuard indent() {
        return GenericGuard([this]() { this->_indent += 1; }, [this]() { this->_indent -= 1; });
    }
};




ASTRenderContext& ASTRenderContext::operator<<(special::Indent const& str) {
    String idt(4 * _indent, ' ');
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, idt.c_str());
    pos += ImVec2(size.x, 0);
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<<(special::BeforeComment const& str) {
    String idt  = "   ";
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, idt.c_str());

    float maybe_x = pos.x + size.x;
    maxcol        = std::max(maybe_x, maxcol);

    pos.x = maxcol;

    _comment = true;
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<<(special::Newline const& str) {
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, " ");

    if (!_comment) {
        maxcol = std::max(pos.x, maxcol);
    }
    _comment = false;

    pos.x = start.x;
    pos.y += (size.y + style->extra_line_space);
    return (*this);
}

void ASTRenderContext::inline_string(const char* str, ImColor color) {
#if 0
    char buffer[64];
    strcpy(buffer, str);
    ImVec2 size;
    ASTInputText(
        "label", // const char* label,
        "hint", // const char* hint,
        (char*)buffer, //char* buf,
        64, //int buf_size,
        size, //const ImVec2& size_arg,
        0, //ImGuiInputTextFlags flags,
        nullptr, //ImGuiInputTextCallback callback,
        nullptr //void* callback_user_data
    );

    _id += 1;
    pos += ImVec2(size.x, 0);
    return;
#else
    ImVec2 size = style->font->CalcTextSizeA(style->font_size, FLT_MAX, 0.0f, str);
    ImRect bb(pos, pos + size);

    unsigned int id = _id;
    ImGui::ItemAdd(bb, id);
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

    ImGuiContext*        g      = ImGui::GetCurrentContext();
    ImGuiInputTextState* state  = ImGui::GetInputTextState(id);
    ImGuiWindow*         window = ImGui::GetCurrentWindow();
    ImGuiIO&             io     = ImGui::GetIO();
    int                  flags;

    if (hovered) {
        g->MouseCursor = ImGuiMouseCursor_TextInput;
    }

    if (g->LastItemData.InFlags & ImGuiItemFlags_ReadOnly)
        flags |= ImGuiInputTextFlags_ReadOnly;

    const bool is_readonly  = (flags & ImGuiInputTextFlags_ReadOnly) != 0;
    const bool is_password  = (flags & ImGuiInputTextFlags_Password) != 0;
    const bool is_undoable  = (flags & ImGuiInputTextFlags_NoUndoRedo) == 0;
    const bool is_resizable = (flags & ImGuiInputTextFlags_CallbackResize) != 0;

    if (is_resizable) {
        // IM_ASSERT(callback != NULL); // Must provide a callback if you set the
        // ImGuiInputTextFlags_CallbackResize flag!
    }

    const bool input_requested_by_nav =
        (g->ActiveId != id) &&
        ((g->NavActivateId == id) && ((g->NavActivateFlags & ImGuiActivateFlags_PreferInput) ||
                                      (g->NavInputSource == ImGuiInputSource_Keyboard)));

    const bool user_clicked       = hovered && io.MouseClicked[0];
    const bool user_scroll_finish = false;
    const bool user_scroll_active = false;
    bool       clear_active_id    = false;
    bool       select_all         = false;

    float scroll_y = FLT_MAX;

    const bool init_reload_from_user_buf = (state != NULL && state->ReloadUserBuf);
    const bool init_changed_specs =
        (state != NULL && state->Stb.single_line != true);  // state != NULL means its our state.
    const bool init_make_active = (user_clicked || user_scroll_finish || input_requested_by_nav);
    const bool init_state       = (init_make_active || user_scroll_active);

    const char* display_str = str;
    float       size_x      = size.x;

    static bool prev = user_clicked;

    if (prev != user_clicked) {
        LOG("make active: " << user_clicked);
        prev = user_clicked;
    }

    if (InputState::state().active_id != id && init_make_active) {
        // IM_ASSERT(state && state->ID == id);
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
        InputState::state().set(id, str);
    }

    if (InputState::state().active_id == id) {
        if (io.InputQueueCharacters.Size > 0) {
            for (int n = 0; n < io.InputQueueCharacters.Size; n++) {
                unsigned int c = (unsigned int)io.InputQueueCharacters[n];
                InputState::state().add_character((int)c);
            }
            io.InputQueueCharacters.resize(0);
        }

        display_str = InputState::state().buffer.data();
        float new_size =
            (size_x / InputState::state().buffer.size()) * InputState::state().buffer.capacity();
        size_x = new_size;
    }

    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    drawlist->AddText(style->font, style->font_size + int(hovered), pos, color, display_str);

    _id += 1;
    pos += ImVec2(size_x, 0);
#endif
}

ASTRenderContext& ASTRenderContext::operator<<(special::Keyword const& keyword) {
    const char* key = keyword.name.c_str();
    inline_string(key, style->keyword);
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<<(special::Docstring const& keyword) {
    String docstring = String("\"\"\"") + keyword.name + String("\"\"\"");

    const char* key = docstring.c_str();
    inline_string(key, style->docstring);
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<<(special::Type const& keyword) {
    const char* key = keyword.name.c_str();
    inline_string(key, style->type);
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<<(const char* str) {
    ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    inline_string(str, ImColor(color.x, color.y, color.z));
    return (*this);
}

ASTRenderContext& ASTRenderContext::operator<<(String const& str) { return (*this) << str.c_str(); }
