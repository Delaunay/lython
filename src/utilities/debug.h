#pragma once

#include <iostream>
#include <tuple>
#include <vector>
#include <unordered_map>

namespace lython {
template<typename T>
struct ToPointer {
    using type = T*;
};

inline 
auto counter() {
    int count = 0;
    return [count] () mutable {
        int prev = count;
        count += 1;
        return prev;
    };
}

template <typename Key, typename Value, typename... Others>
std::ostream& operator<<(std::ostream& out,  std::unordered_map<Key, Value, Others...> const& hashtable);

template <typename T>
void println(std::ostream& out, T const& val) {
    print(out, val);
    out << "\n";
}

template <std::size_t I = 0, typename... Ts>
void printTuple(std::ostream& out, const std::tuple<Ts...>& tuple) {
    if constexpr (I < sizeof...(Ts)) {
        if (I > 0) {
            out << ", ";
        }
        out << std::get<I>(tuple);
        printTuple<I + 1>(out, tuple);    // Recursively call for the next element
    }
}

template <typename... Ts>
std::ostream& operator<<(std::ostream& out, const std::tuple<Ts...>& tuple) {
    out << "(";
    printTuple<0>(out, tuple);
    return out << ")";
}

template <typename T, typename... Others>
std::ostream& operator<<(std::ostream& out, const std::vector<T, Others...>& vec) {
    out << "[";
    auto cnt = counter();

    for (const T& element : vec) {
        if (cnt() > 0) {
            out << ", ";
        }
        out << element;
    }
    return out << "]";
}

template <typename Key, typename Value, typename... Others>
std::ostream& operator<<(std::ostream& out, std::unordered_map<Key, Value, Others...> const& hashtable) {
    out << "{";
    auto cnt = counter();

    for (auto const& pair : hashtable) {
        if (cnt() > 0) {
            out << ", ";
        }
        out << pair.first;
        out << ": ";
        out << pair.second;
    }
    return out << "}";
}


template <typename T>
void showvar(std::ostream& out, int line, const char* name, T const& val) {
    out << "L";
    (out << line);
    out << " ";
    (out << name);
    out << ": ";
    (out << val);
    out << "\n";
}

#define LYT_SHOW(out, x) showvar(out, __LINE__, #x, x)

}