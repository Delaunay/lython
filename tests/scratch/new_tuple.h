


//
// This create a bigger than necessary tuple because of padding
//
template<typename... Args>
struct RecTuple;

template<typename Head, typename... Tail>
struct RecTuple<Head, Tail...> {
    Head head;
    RecTuple<Tail...> tail;

    template<typename... Args>
    RecTuple(Args&&... args) : head(std::forward<Args>(args)...) {}

    static constexpr std::size_t size() {
        return sizeof...(Tail) + 1;
    }

    template<typename T>
    T& get() {
        return getHelper<T>(*this);
    }

private:
    template<typename T, typename First, typename... Rest>
    T& getHelper(RecTuple<First, Rest...>& tuple) {
        if constexpr (std::is_same_v<T, First>) {
            return tuple.head;
        } else {
            return getHelper<T>(tuple.tail);
        }
    }
};

template<>
struct RecTuple<> {};


//
// This create the right size tuple
// but handling non trivially copyable value AND keeping the tuple
// trivially copyable when all the args are trivially copyable is annoying
//

template <typename T, typename... Rest>
static constexpr std::size_t sizeof_args() {
    if constexpr (sizeof...(Rest) == 0) {
        return sizeof(T);
    } else {
        return sizeof(T) + sizeof_args<Rest...>();
    }
}

template <std::size_t N, typename... TupleArgs>
constexpr std::size_t get_element_offset() {
    if constexpr (N == 0) {
        return 0;
    } else {
        return sizeof_args<TupleArgs...>() -
               sizeof_args<std::tuple_element_t<N, std::tuple<TupleArgs...>>>() +
               get_element_offset<N - 1, TupleArgs...>();
    }
}

// This is trivially copyable but it does not copy the non trivially copyable value well
template <typename... Args>
struct MyTuple {
    int8 memory[sizeof_args<Args...>()];

    MyTuple() { std::memset(memory, 0, sizeof(memory)); }

    // Initialize memory with the arguments
    MyTuple(Args... args) {
        std::size_t offset = 0;
        set_from_args(offset, args...)
    }

    template <std::size_t N>
    auto get() const {
        static_assert(N < sizeof...(Args), "Index out of bounds");
        using ElementType = std::tuple_element_t<N, std::tuple<Args...>>;
        return *reinterpret_cast<const ElementType*>(memory + get_element_offset<N>());
    }

    template <typename T, typename... Rest>
    void set_from_args(std::size_t& offset, T arg, Rest... rest) {
        if constexpr (std::is_trivially_copyable_v<T>) {
            std::memcpy(memory + offset, &arg, sizeof(T));
        } else {
            new (memory + offset) T(arg);
        }
        offset += sizeof(T);
        set_from_args(offset, rest...);
    }

    void set_from_args(std::size_t& offset) {}
};
