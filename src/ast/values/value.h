#pragma once

#include "dtypes.h"

namespace lython {

struct Value;

#define VALUE_TYPES(X)     \
    X(bool, i1)      \
    X(uint64, u64)   \
    X(int64, i64)    \
    X(uint32, u32)   \
    X(int32, i32)    \
    X(uint16, u16)   \
    X(int16, i16)    \
    X(uint8, u8)     \
    X(int8, i8)      \
    X(float32, f32)  \
    X(float64, f64)  \
    X(Function, fun) \
    X(None, none)


struct Error { };

template <typename T>
struct Getter {
    static T get(Value& v, Error& err);
};



//
// Simple dynamic value that holds small value on the stack
// and objects on the heap, the value is cheap to copy.
// it does not handle auto deletion (like a variant might)
//
// struct smaller than 64bit (sizeof(Value::Holder)) are held in the stack
// anything over that heap is allocated to hold them
//
// if we add Vec3 this will be able to hold even bigger things on the stack
// We migt want to reduce the overhead of tagging the struct by repurposing
// some bit of the value as tag bit => we cannot do this anymore because
// the tag is not bounded as it used to be.
// Unless we split custom object as their own value type.
// but we cannot tag floats, unless we use a different type
//
// Color (rgba  uin8 * 4) =>  32          + tag => 64
// Color (       f32 * 4) => 128 | 256          => 160 | 288
// Vec3  (       f32 * 3) =>  96 | 192          => 128 | 224 => expected 16bytes but got 24 bytes
// Transform3D (f32 * 16) => 512 | 1024
// Transform2D (f32 *  9) => 288 | 576
// Array                  => 256         <= No trivially copyable
//
// Probably that we should stop at 256 in size
//
//  If we add vec3 `Holder` gets padded from 12 to 16
//  so we can also add vec4 without any drawbacks
//
//
// |   Padding   | no  | vec3 | vec4 |
// | Holder      | 0   | 4    | 0    |
// | Value       | 4   | 4    | 4    |
//
// | Min Size    | no  | vec3 | vec4 |
// | Holder      | 8   | 12   | 16   |
// | Value       | 12  | 16   | 20   |
//
// | Final Size  | no  | vec3 | vec4 |
// | Holder      | 8   | 16   | 16   |
// | Value       | 16  | 24   | 24   |
//
// | Water Space | no  | vec3 | vec4 |
// | Holder      | 0%  | 25%  | 0%   |
// | Value       | 33% | 50%  | 20%  | <= Hurts that 1/3 is wasted
//
// We have space for bit flags
//   - int immutable:1;
//   -
struct Value {
    union Holder {
    #define ATTR(type, name) type name;
        TYPES(ATTR)
    #undef ATTR

        void* obj;
    };

    uint32 tag;
    Holder value;

    Value(): tag(type_id<None>()) {}

#define CTOR(type, name) \
    Value(type name): tag(type_id<type>()) { value.name = name; }

    TYPES(CTOR)
#undef CTOR

    Value(int tag, void* ptr): tag(tag) { value.obj = ptr; }

    bool is_type(int obj_type_id) const { return obj_type_id == tag; }

    template <typename T>
    bool is_type() const {
        return is_type(type_id<T>());
    }

    template <typename T>
    T as() {
        return Getter<T>::get(*this);
    }

    template <typename T>
    T as() const {
        return Getter<T>::get(*this);
    }

    bool is_object() const { return tag >= int(ValueTypes::Max); }

    template <typename T>
    static bool is_allocated() {
        return !(sizeof(T) <= sizeof(Value::Holder) && std::is_trivially_copyable<T>::value);
    }

    template <typename T>
    void* get_storage() {
        if (is_object()) {
            if (!is_allocated<T>()) {
                void* mem = reinterpret_cast<uint8*>(this) + offsetof(Value, value);
                return mem;
            }
            // object was too big and had to be allocated
            return this->value.obj;
        }
        return nullptr;
    }
};


//
// Getter
//
template <typename T>
T Getter<T>::get(Value& v, Error& err) {
    using Underlying = std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<T>>>;
    static T def = nullptr;

    if (v.tag == type_id<Underlying>()) {
        void* ptr = v.get_storage<Underlying>();
        return static_cast<T>(ptr);
    }

    return def;
}

#define GETTER(type, name)                                  \
template <>                                             \
struct Getter<type> {                                   \
    static type get(Value& v, Error& err) { return v.value.name; }; \
};

TYPES(GETTER)
#undef GETTER


//
// ostream
//

std::ostream& operator<<(std::ostream& os, None const& v) { return os << "None"; }

std::ostream& operator<<(std::ostream& os, Value const& v) {
    switch (ValueTypes(v.tag)) {
    #define CASE(type, name) \
    case ValueTypes::name: return os << "Value(" << v.value.name << ": " #type << ")";
        TYPES(CASE)
    #undef CASE

    case ValueTypes::Max: break;
    }

    return os << "obj";
}

}