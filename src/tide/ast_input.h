#pragma once

#include <iostream>

#include "dtypes.h"

namespace lython {

struct InputState {
    String buffer;

    int active_id;

    void set(int id, const char* str) {
        active_id = id;
        int newsize = std::max(int(buffer.size()), int(strlen(str) * 1.25));
        buffer.reserve(newsize);
        memset(buffer.data(), '\0', newsize);
        buffer += str;
        std::cout << "bufer " << buffer << std::endl;
    }

    void add_character(int c) {
        buffer.push_back(c);
    }

    static InputState& state();
};

}