#ifndef LYTHON_OBJECT_VALUE_HEADER
#define LYTHON_OBJECT_VALUE_HEADER

// Actual method signature is WIP
// most mght become references

#include "ast/constant.h"
#include "dtypes.h"
#include "native.h"

namespace lython {

/* Compile time representation of an object
 * This is just an array of attributes, methods were moved out during SEMA
 *
 * This is a compile time representation during macro expension
 */
struct Object: public NativeObject {
    Array<struct Constant*> attributes;
    // Methods belong in the class
    // The object type is not known
    // we rely on the SEMA to guarante the type

    Object():
        NativeObject(meta::type_id<Object>())
    {}

    bool is_native() const override { return false; }
    bool is_pointer() const override { return false; }
    bool is_valid() const override { return true; }
    int8* _memory() override { return nullptr; }
};

// Same as an object, but allocate a single block for all the attributes
// each attribute is a native value, avoids the ConstantValue overhead
// useful for POD types
struct PackedObject: public NativeObject {
    int8* data;

    PackedObject():
        NativeObject(meta::type_id<PackedObject>())
    {}

    bool is_native() const override { return false; }
    bool is_pointer() const override { return false; }
    bool is_valid() const override { return true; }
    int8* _memory() override { return nullptr; }

    template <typename T>
    T& attribute(int offset) {
        T* val = static_cast<T*>(data + offset);
        return *val;
    }
};

}  // namespace lython

#endif