#include <iostream>
#include <tuple>
#include <vector>
#include <unordered_map>

template<typename T>
struct ToPointer {
    using type = T*;
};

auto counter() {
    int count = 0;
    return [count] () mutable {
        int prev = count;
        count += 1;
        return prev;
    };
}

template <typename Key, typename Value, typename... Others>
void print(std::ostream& out, std::unordered_map<Key, Value, Others...> const& hashtable);


template <typename T>
void print(std::ostream& out, T& val) {
    out << val;
}

template <typename T>
void print(std::ostream& out, T const& val) {
    out << val;
}


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

        print(out, std::get<I>(tuple));
        printTuple<I + 1>(out, tuple);    // Recursively call for the next element
    }
}

template <typename... Ts>
void print(std::ostream& out, const std::tuple<Ts...>& tuple) {
    out << "(";
    printTuple<0>(out, tuple);
    out << ")";
}

template <typename T, typename... Others>
void print(std::ostream& out, const std::vector<T, Others...>& vec) {
    out << "[";
    auto cnt = counter();

    for (const T& element : vec) {
        if (cnt() > 0) {
            out << ", ";
        }

        print(out, element);
    }
    out << "]";
}

template <typename Key, typename Value, typename... Others>
void print(std::ostream& out, std::unordered_map<Key, Value, Others...> const& hashtable) {
    out << "{";
    auto cnt = counter();

    for (auto const& pair : hashtable) {
        if (cnt() > 0) {
            out << ", ";
        }
        print(out, pair.first);
        out << ": ";
        print(out, pair.second);
    }
    out << "}";
}


template <typename T>
void showvar(std::ostream& out, int line, const char* name, T const& val) {
    out << "L";
    print(out, line);
    out << " ";
    print(out, name);
    out << ": ";
    print(out, val);
    out << "\n";
}

#define LYT_SHOW(out, x) showvar(out, __LINE__, #x, x)
