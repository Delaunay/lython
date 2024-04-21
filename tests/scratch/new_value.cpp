
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

#define KIWI_SVO 1

template <typename V>
using Array = std::vector<V>;

template <typename... Args>
using Tuple = std::tuple<Args...>;

using String = std::string;

struct Value;

using uint64   = std::uint64_t;
using int64    = std::int64_t;
using uint32   = std::uint32_t;
using int32    = std::int32_t;
using uint16   = std::uint16_t;
using int16    = std::int16_t;
using uint8    = std::uint8_t;
using int8     = std::int8_t;
using float32  = float;
using float64  = double;
using Function = Value (*)(void*, Array<Value>&);

struct Struct {
    void* ptr;

    // we could save the deleter here and remove ManagedVariable
    // at the price of a bigger overhead
    // void(*deleter)(void*) = nullptr
};

struct None {};

struct Vec3 {
    float x, y, z;
};

struct Vec4 {
    float x, y, z, d;
};

#define TYPES(X)     \
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

//                  \
    // X(Vec3, vec3)    \
    // X(Vec4, vec4)    \

enum class ValueTypes
{
#define ENUM(type, name) name,
    TYPES(ENUM)
#undef ENUM
    Max
};

#if 1
inline int _new_type() {
    static int counter = int(ValueTypes::Max);
    return ++counter;
}

template <typename T>
struct _type_id {
    static int id() {
        static int _id = _new_type();
        return _id;
    }
};

//
// Small type reserve their ID
//
#define TYPEID_SPEC(type, name)                                     \
    template <>                                                     \
    struct _type_id<type> {                                         \
        static constexpr int id() { return int(ValueTypes::name); } \
    };

TYPES(TYPEID_SPEC)
#undef TYPEID_SPEC

template <typename T>
int type_id() {
    return _type_id<T>::id();
}

#else
template <typename T>
constexpr int type_id() {
    return reinterpret_cast<uint64>(&type_id<T>);
}
#endif

template <typename T>
struct Getter {
    static T get(Value& v);
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
        // return sizeof(T) > sizeof(Value::Holder) || !std::is_trivially_copyable<T>::value;
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

template <typename T>
T Getter<T>::get(Value& v) {
    using Underlying = std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<T>>>;
    ;
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
        static type get(Value& v) { return v.value.name; }; \
    };

TYPES(GETTER)
#undef GETTER

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



//
// Not convinced this is useful
//
// In our use case we manage the lifetime of the value so
// there might be a use to store the deleter along side the value
// but calling it in the destructor is not that interesting
//
//
struct ManagedValue {
    Value                      val;
    std::function<void(Value)> deleter;
    bool                       valid;

    void destroy() {
        if (valid) {
            valid = false;
            deleter(val);
        }
    }
};

template <typename T, typename... Args>
ManagedValue make_managed_value(Args... args) {
    auto [v, d] = make_value<T>(args...);
    return ManagedValue{v, d, true};
}

//
// Example
//






struct ScriptObject {
    // Name - Value
    // We would like to get rid of the name if possible
    // during SEMA we can resolve the name to the ID
    // so we would never have to lookup by name
    // If some need the name at runtime this is reflection stuff
    // and that would be handled by a different datastructure
    // for now it will have to do
    //
    // Usually methods will not be stored there
    // but it can happen when the code assign method as attributes
    //
    Array<Tuple<String, Value>> attributes;

    ~ScriptObject() {
        std::cout << "Destructor called"
                  << "\n";
    }
};

// We don't check the typeid because all the runtime object use the same underlying type
template <>
struct Getter<ScriptObject*> {
    static ScriptObject* get(Value& v) { return reinterpret_cast<ScriptObject*>(v.value.obj); };
};

//



// Trivial
template <typename T, int N>
struct MyArray {
    T values[N];
};
