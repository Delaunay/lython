#ifndef LYTHON_MAGIC_HEADER
#define LYTHON_MAGIC_HEADER

#include "../dtypes.h"
#include <iostream>

namespace lython {

template <typename T>
void print(T const& obj, std::ostream& out = std::cout) {
    obj.print(out);
}

template <>
inline void print(String const& obj, std::ostream& out) {
    out << obj;
}

template <>
inline void print(int const& obj, std::ostream& out) {
    out << obj;
}

template <typename T>
void print(T* const& obj, std::ostream& out = std::cout) {
    obj->print(out);
}

template <typename T>
String str(T const& obj) {
    StringStream ss;
    print(obj, ss);
    return ss.str();
}

template <typename T>
String str(T* const& obj) {
    if (obj == nullptr) {
        return "None";
    }
    return obj->__str__();
}

inline String str(String const& obj) { return obj; }

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
