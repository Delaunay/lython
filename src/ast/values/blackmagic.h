#include <type_traits>

// -ftemplate-depth=1024
template <std::size_t N> struct ArrayStruct {
  static_assert(N <= 1024, "Hitting template recursion limit");

  char head;
  ArrayStruct<N - 1> tail;

  template <std::size_t I> char &get() { return tail.template get<I - 1>(); }

  template <> char &get<0>() { return head; }
};

template <> struct ArrayStruct<1> {
};

template <typename T, typename... Args> struct ArgumentByteSize {
  enum { value = sizeof(T) + ArgumentByteSize<Args...>::value };
};

template <typename T> struct ArgumentByteSize<T> {
  enum { value = sizeof(T) };
};

template <typename T, typename... Args> struct ArgumentCount {
  enum { value = 1 + ArgumentCount<Args...>::value };
};

template <typename T> struct ArgumentCount<T> { 
  enum { value = 1 };
};

template <typename... Args> void ArgPacking(Args... args) {
  using ArgStruct = ArrayStruct<ArgumentByteSize<Args...>::value>;

  ArgStruct PackedArgs;
  
}

