#ifndef LYTHON_UTILITIES_GUARD_HEADER
#define LYTHON_UTILITIES_GUARD_HEADER

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

#endif
