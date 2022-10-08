#ifndef LYTHON_UTILITIES_GUARD_HEADER
#define LYTHON_UTILITIES_GUARD_HEADER

namespace lython {
// Execute function upon destruction
template <typename Exit>
struct Guard {
    Guard(Exit fun): on_exit(fun) {}

    ~Guard() { on_exit(); }

    Exit on_exit;
};

template <typename Exit>
Guard<Exit> guard(Exit fun) {
    return Guard(fun);
}

template <typename T, typename U>
struct PopGuard {
    PopGuard(T& array, U const& v): array(array), oldsize(array.size()) { array.push_back(v); }

    PopGuard(T& array): array(array), oldsize(array.size()) {}

    ~PopGuard() { array.pop_back(); }

    U const& last(int offset, U const& default_value) const {
        if (oldsize >= offset) {
            return array[oldsize - offset];
        }
        return default_value;
    }

    T&          array;
    std::size_t oldsize;
};
}  // namespace lython

#endif
