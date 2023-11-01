#pragma once

template <typename T, typename N>
bool in(T const& e, N const& v) {
    return e == v;
}

template <typename T, typename N, typename... Args>
bool in(T const& e, N const& v, Args... args) {
    return e == v || in(e, args...);
}

template <typename T, typename... Args>
bool in(T const& e, Args... args) {
    return in(e, args...);
}
