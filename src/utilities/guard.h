#ifndef LYTHON_UTILITIES_GUARD_HEADER
#define LYTHON_UTILITIES_GUARD_HEADER

namespace lython {
// Execute function upon destruction
template <typename Exit, typename... Args>
struct Guard {
    Guard(Exit fun, Args... args): fun_args(std::make_tuple(args...)), on_exit(fun) {}

    ~Guard() { 
        std::apply(on_exit, fun_args);
    } 

    Tuple<Args...> fun_args;
    Exit on_exit;
};

template <typename Exit, typename... Args>
Guard<Exit, Args...> guard(Exit fun, Args... args) {
    return Guard<Exit, Args...>(fun, args...);
}


#define KW_PP_THIRD_ARG(a,b,c,...) c
#define KW_VA_OPT_SUPPORTED_I(...) KW_PP_THIRD_ARG(__VA_OPT__(,),true,false,)
#define KW_VA_OPT_SUPPORTED KW_VA_OPT_SUPPORTED_I(?)



#define KW_STR_(x, y) x ## y
#define KW_STR(x, y) KW_STR_(x, y)
#define KW_IDT(name) KW_STR(name, __LINE__)

#if KW_VA_OPT_SUPPORTED
#define KW_DEFERRED(fun, ...) \
    auto KW_IDT(_) = guard(fun __VA_OPT__(,) __VA_ARGS__);
#else
#define KW_DEFERRED(fun, ...) \
    auto KW_IDT(_) = guard(fun, ##__VA_ARGS__);
#endif

template<typename T>
struct ElementProxy {
    int i;
    Array<T>* holder;

    T& operator->() {
        return (*holder)[i];
    }
};

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
