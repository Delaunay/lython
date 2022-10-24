#ifndef LYTHON_OBJECT_VALUE_HEADER
#define LYTHON_OBJECT_VALUE_HEADER

// Actual method signature is WIP
// most mght become references

#include "ast/constant.h"
#include "dtypes.h"
#include "native.h"

namespace lython {

/* Compile time representation of an object
 * This is just an array of attributes, methods were move out during SEMA
 */
struct Object: public NativeObject {
    Array<ConstantValue> attributes;
};

// Same as an object, but allocate a single block for all the attributes
// each attribute is a native value, avoids the ConstantValue overhead
// useful for POD types
struct PackedObject: public NativeObject {
    int8* data;

    template <typename T>
    T& attribute(int offset) {
        T* val = static_cast<T*>(data + offset);
        return *val;
    }
};

}  // namespace lython

#endif