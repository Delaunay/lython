#pragma once

#include "Types.h"
#include <string>
#include <cmath>

namespace lython
{
inline
String align_left(const String & name, int8 col){
    int32 name_size = int32(name.size());

    if (name_size > col)
        return name.substr(0, std::size_t(col));

    if (name_size == col)
        return name;

    return name + String (std::size_t(col - name_size), ' ');
}

inline
String align_right(const String & name, int8 col){
    int32 name_size = int32(name.size());

    if (name_size > col)
        return name.substr(0, std::size_t(col));

    if (name_size == col)
        return name;

    return String (std::size_t(col - name_size), ' ') + name;
}

inline
String align_center(const String & name, int8 col){
    int32 name_size = int32(name.size());

    if (name_size > col)
        return name.substr(0, std::size_t(col));

    if (name_size == col)
        return name;

    int32 n = col - name_size;
    n = (n + n % 2)/ 2;

    return String (std::size_t(n), ' ') + name + String (std::size_t(n), ' ');
}

template<typename T>
String to_string(T nb, int32 b){
    int32 n = std::log10(nb) + 1;

    if (nb == 0)
        n = 1;

    n = b - n;
    if (n > 0){
        auto str = std::string(n, ' ') + std::to_string(nb);
        return String(str.c_str());
    }
    return String(std::to_string(nb).c_str());
}

inline
String to_string(float32 nb, int32 b){
    int32 n = int32(std::log10(nb)) + 2;

    n = b - n;
    if (n > 0){
        auto str = std::string(std::size_t(n), ' ') + std::to_string(nb);
        return String(str.c_str());
    }

    return String(std::to_string(nb).c_str());
}

inline
String to_string(float64 nb, int32 b){
    int32 n = int32(std::log10(nb)) + 2;
    n = b - n;
    if (n > 0) {
        auto str =  std::string(std::size_t(n), ' ') + std::to_string(nb);
        return String(str.c_str());
    }
    return String(std::to_string(nb).c_str());
}
}
