#pragma once

#include "Types.h"
#include <string>
#include <cmath>

namespace lython
{
inline
std::string align_left(const std::string& name, uint8 col){
    if (name.size() > col)
        return name.substr(0, col);

    if (name.size() == col)
        return name;

    return name + std::string(col - name.size(), ' ');
}

inline
std::string align_right(const std::string& name, uint8 col){
    if (name.size() > col)
        return name.substr(0, col);

    if (name.size() == col)
        return name;

    return std::string(col - name.size(), ' ') + name;
}

inline
std::string align_center(const std::string& name, uint8 col){
    if (name.size() > col)
        return name.substr(0, col);

    if (name.size() == col)
        return name;

    uint8 n = col - name.size();
    n = (n + n % 2)/ 2;

    return std::string(n, ' ') + name + std::string(n, ' ');
}

template<typename T>
std::string to_string(T nb, int32 b){

    int32 n = std::log10(nb) + 1;

    if (nb == 0)
        n = 1;

    n = b - n;
    if (n > 0)
        return std::string(n, ' ') + std::to_string(nb);
    return std::to_string(nb);
}

inline
std::string to_string(float32 nb, int32 b){
    int32 n = std::log10(nb) + 2;
    n = b - n;
    if (n > 0)
        return std::string(n, ' ') + std::to_string(nb);
    return std::to_string(nb);
}

inline
std::string to_string(float64 nb, int32 b){
    int32 n = std::log10(nb) + 2;
    n = b - n;
    if (n > 0)
        return std::string(n, ' ') + std::to_string(nb);
    return std::to_string(nb);
}

}
