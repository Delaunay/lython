#ifndef LYTHON_MAGIC_HEADER
#define LYTHON_MAGIC_HEADER

#include "../dtypes.h"
#include <iostream>

namespace lython {

template <typename T>
void print(T const &obj, std::ostream &out = std::cout) {
    obj.print(out);
}

template <typename T>
void print(T *const &obj, std::ostream &out = std::cout) {
    obj->print(out);
}

template <typename T>
String str(T const &obj) {
    return obj.__str__();
}

template <typename T>
String str(T *const &obj) {
    return obj->__str__();
}

inline String str(String const &obj) { return obj; }

} // namespace lython

#endif
