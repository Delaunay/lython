#ifndef LYTHON_MAGIC_HEADER
#define LYTHON_MAGIC_HEADER

#include "../dtypes.h"
#include <iostream>

namespace lython {

template <typename T>
void print(T const &obj, std::ostream &out = std::cout) {
    obj.print(out);
}

template <>
inline void print(String const &obj, std::ostream &out) {
    out << obj;
}

template <typename T>
void print(T *const &obj, std::ostream &out = std::cout) {
    obj->print(out);
}

template <typename T>
String str(T const &obj) {
    StringStream ss;
    print(obj, ss);
    return ss.str();
}

template <typename T>
String str(T *const &obj) {
    if (obj == nullptr) {
        return "None";
    }
    return obj->__str__();
}

inline String str(String const &obj) { return obj; }

} // namespace lython

#endif
