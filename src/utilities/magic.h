#ifndef LYTHON_MAGIC_HEADER
#define LYTHON_MAGIC_HEADER

#include "../dtypes.h"
#include <iostream>

namespace lython {


#define KW_IS_EXPR_VALID(name, EXPR)                                    \
    template <typename T, typename = void>                              \
    struct has_ ## name: std::false_type {};                            \
    template <typename T>                                               \
    struct has_ ## name <T, decltype(EXPR, void())> : std::true_type {};

#define KW_DEFINE_HAS_METHOD(name, method) \
    KW_IS_EXPR_VALID(name, std::declval<T>().method())


KW_DEFINE_HAS_METHOD(len, __len__)
KW_DEFINE_HAS_METHOD(str, __str__)
KW_DEFINE_HAS_METHOD(repr, __repr__)
KW_DEFINE_HAS_METHOD(size, size)

KW_IS_EXPR_VALID(ostream, std::declval<std::ostream&>() << std::declval<T>())


// Helper output to anything but std::cout by default
template <typename T>
void print(T const& obj, std::ostream& out = std::cout) {
    constexpr bool impl_str = has_str<T>::value;
    constexpr bool impl_os = has_ostream<T const&>::value;

    if constexpr (impl_str) {
        out << obj.__str__();
        return;
    }
    
    if constexpr (impl_os) {
        out << obj;
        return;
    } 

    static_assert(impl_str || impl_os, "Could not find a convert function");
    //obj.print(out);
}

template <typename T>
void print(T* const& obj, std::ostream& out = std::cout) {
    print(*obj, out);
}

// Helper Output to string
template <typename T>
String str(T const& obj) {
    constexpr bool impl_str = has_str<T>::value;
    constexpr bool impl_os = has_ostream<T const&>::value;

    if constexpr (impl_str) {
        return obj.__str__();
    }
    if constexpr (impl_os) {
        // C++ operator
        StringStream ss;
        ss << obj;
        return ss.str();
    } 
    
    static_assert(impl_str || impl_os, "Could not find a convert function");
}


// debug print, prints more info
template <typename T>
String repr(T const& obj) {
    if constexpr (has_repr<T>::value) {
        return obj.__repr__();
    }

    StringStream ss;
    ss << meta::type_name<T>();
    ss << "(" << str(obj) << ")"; 
    return ss.str();
}


inline String str(std::string const& s) { return String(s.c_str()); }

template <typename T>
String str(T* const& obj) {
    if (obj == nullptr) {
        return "None";
    }
    return str(*obj);
}

inline String str(String const& obj) { return obj; }

// makes idx - 1 well defined
template <typename T, typename IndexType = int>
int len(T const& val) {
    constexpr bool impl_len = has_len<T>::value;
    constexpr bool impl_sz = has_size<T const&>::value;

    if constexpr (impl_len) {
        return IndexType(val.__len__());
    } 
    
    if constexpr (impl_sz) {
        return IndexType(val.size());
    } 

    static_assert(impl_sz || impl_len, "Object does not provide len method");
}

#define BINARY(X)                      \
    X(+, __add__(self, other))         \
    X(-, __sub__(self, other))         \
    X(*, __mul__(self, other))         \
    X(/, __truediv__(self, other))     \
    X(\/\/, __floordiv__(self, other)) \
    X(%, __mod__(self, other))         \
    X(**, __pow__(self, other))        \
    X(>>, __rshift__(self, other))     \
    X(<<, __lshift__(self, other))     \
    X(&, __and__(self, other))         \
    X(|, __or__(self, other))          \
    X(^, __xor__(self, other))

#define COMP(X)                \
    X(<, __lt__(self, other))  \
    X(>, __gt__(self, other))  \
    X(<=, __le__(self, other)) \
    X(>=, __ge__(self, other)) \
    X(==, __eq__(self, other)) \
    X(!=, __ne__(self, other))

#define AUG(X)                           \
    X(-=, __isub__(self, other))         \
    X(+=, __iadd__(self, other))         \
    X(*=, __imul__(self, other))         \
    X(/=, __idiv__(self, other))         \
    X(\/\/=, __ifloordiv__(self, other)) \
    X(%=, __imod__(self, other))         \
    X(* *=, __ipow__(self, other))       \
    X(>>=, __irshit__(self, other))      \
    X(<<=, __ilshit__(self, other))      \
    X(&=, __iand__(self, other))         \
    X(|=, __ior__(self, other))          \
    X(^=, __ixor__(self, other))

#define UNARY(X)               \
    X(-, __neg__(self, other)) \
    X(+, __pos__(self, other)) \
    X(~, __invert__(self, other))

}  // namespace lython

#endif
