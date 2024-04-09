#pragma once

#include "utilities/metadata.h"

#define KIWI_SVO 1

namespace lython {

struct Value;

struct Color {
    uint8 r;
    uint8 g;
    uint8 b;
    uint8 a;

    bool operator== (Color const& val) const {
        return *reinterpret_cast<uint32 const*>(this) == *reinterpret_cast<uint32 const*>(&val);
    }  
};

struct Pointi {
    int32 x;
    int32 y;

    bool operator== (Pointi const& val) const {
        return *reinterpret_cast<uint64 const*>(this) == *reinterpret_cast<uint64 const*>(&val);
    }  
};

struct Pointf {
    float32 x;
    float32 y;

    bool operator== (Pointf const& val) const {
        return (x == val.x) && (y == val.y);
    }  
};


struct GetterError {
    int failed            = 0;
    int value_type_id     = -1;
    int requested_type_id = -1;
};

template <typename T>
struct Getter {
    static T       get(Value& v, GetterError& err);
    static const T get(const Value& v, GetterError& err);
};

template <typename T>
struct Query {
    static bool get(const Value& v);
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
//
// We could have a version that removes the tag
// for speeding up execution more (once SEMA is mature enough)
struct Value {
    union Holder {
#define ATTR(type, name) type name;
        KIWI_VALUE_TYPES(ATTR)
#undef ATTR

        void* obj;
    };

    // The VM might not need the type tag
    // once sema is passed we should be able to guarantee
    // the oprations are ok
    std::conditional_t<true, uint32, void> tag;
    Holder                                 value;

    Value(): tag(meta::type_id<_Invalid>()) {}

    // destroy the value using its tag to lookup the appropriate destructor
    bool  destroy();
    Value copy() const;
    // useful to make a reference to something that is usually copied (like an int)
    Value       ref();
    std::size_t hash() const;

#define CTOR(type, name)                                        \
    Value(type name): tag(meta::type_id<type>()) {              \
        static_assert(std::is_trivially_copyable<type>::value); \
        value.name = name;                                      \
    }

    KIWI_VALUE_TYPES(CTOR)
#undef CTOR

    Value(int tag, void* ptr): tag(tag) { value.obj = ptr; }

    bool is_type(int obj_type_id) const { return obj_type_id == tag; }

    template <typename T>
    bool is_type() const {
        return is_type(meta::type_id<T>());
    }

    template <typename T>
    bool operator==(T const& val) const {
        if (tag == meta::type_id<T>()) {
            return as<T>() == val;
        }
        return false;
    }

    template <typename T>
    bool operator!=(T const& val) const {
        return !((*this) == val);
    }

    bool operator==(Value const& val) const;

    bool operator!=(Value const& val) const { return !((*this) == val); }

    template <typename T>
    bool is_valid() const {
        return Query<T>::get(*this);
    }

    static GetterError global_err;

    static void reset_error() {
        global_err.failed            = 0;
        global_err.requested_type_id = -1;
        global_err.value_type_id     = -1;
    }

    static bool has_error() { return global_err.failed > 0; }

    template <typename T>
    T as(GetterError& error = global_err) {
        return Getter<T>::get(*this, error);
    }

    template <typename T>
    T as(GetterError& error = global_err) const {
        return Getter<T>::get(*this, error);
    }

    template <typename T>
    T& ref(GetterError& error = global_err) {
        return *as<T*>(error);
    }

    template <typename T>
    T const& ref(GetterError& error = global_err) const {
        return *as<T*>(error);
    }

    template <typename T>
    static bool is_allocated() {
        return !is_small<T>();
    }

    template <typename T>
    static constexpr bool is_small() {
        return (sizeof(T) <= sizeof(Value::Holder) && std::is_trivially_copyable<T>::value);
    }

    // This return a pointer to the storage
    // Type
    // int     =>  int*
    // int*    =>  int**
    // String  =>  String*
    template <typename T>
    T* pointer() {
        // The pointer to the data is stored inside itself
        if constexpr (is_small<T>()) {
            return reinterpret_cast<T*>(&value);
        } else {
            // The data is stored in dynamically allocated memory
            return reinterpret_cast<T*>(value.obj);
        }
    }

    template <typename T>
    T const* pointer() const {
        // The pointer to the data is stored inside itself
        if constexpr (is_small<T>()) {
            return reinterpret_cast<T const*>(&value);
        } else {
            // The data is stored in dynamically allocated memory
            return reinterpret_cast<T const*>(value.obj);
        }
    }

    std::ostream& print(std::ostream& out) const;
    std::ostream& debug_print(std::ostream& os) const;
};

//
// Getter
//
template <typename T>
T Getter<T>::get(Value& v, GetterError& err) {
    // T const * const&
    using NoConst   = std::remove_const_t<std::remove_reference_t<T>>;      // T const& => T
    using NoPointer = std::remove_const_t<std::remove_pointer_t<NoConst>>;  // T const* => T

    if constexpr (std::is_reference<T>::value) {
        return *v.as<NoConst*>(err);
    } else {
        // Storing int, we want int
        if (v.tag == meta::type_id<NoConst>()) {
            NoConst* ptr = v.pointer<NoConst>();
            return *ptr;
        }

        // Storing int*, we want int
        if (v.tag == meta::type_id<NoConst*>()) {
            NoConst** ptr = v.pointer<NoConst*>();
            return **ptr;
        }

        // Storing int, we want int*
        if constexpr (std::is_pointer_v<NoConst>) {
            if (v.tag == meta::type_id<NoPointer>()) {
                NoPointer* ptr = v.pointer<NoPointer>();
                return ptr;
            }
        }

        err.failed += 1;
        err.value_type_id     = v.tag;
        err.requested_type_id = meta::type_id<T>();
        return T();
    }
}

template <typename T>
T const Getter<T>::get(Value const& v, GetterError& err) {
    using NoConst   = std::remove_const_t<std::remove_reference_t<T>>;
    using NoPointer = std::remove_const_t<std::remove_pointer_t<NoConst>>;

    if constexpr (std::is_reference<T>::value) {
        return *v.as<NoConst const*>(err);
    } else {
        err.failed = false;
        // Storing int, we want int
        if (v.tag == meta::type_id<NoConst>()) {
            NoConst const* ptr = v.pointer<NoConst>();
            return *ptr;
        }

        // is this possible ? we are returning a copy anyway
        // Storing int*, we want int
        if (v.tag == meta::type_id<NoConst*>()) {
            NoConst const* const* ptr = v.pointer<NoConst const*>();
            return **ptr;
        }

        if constexpr (std::is_pointer_v<NoConst>) {
            // Storing int*, we want int const*
            if (v.tag == meta::type_id<NoPointer*>()) {
                NoPointer const* ptr = v.pointer<NoPointer const>();
                return ptr;
            }

            // Storing int, we want int*
            if (v.tag == meta::type_id<NoPointer>()) {
                NoPointer const* ptr = v.pointer<NoPointer const>();
                return ptr;
            }
        }

        err.failed += 1;
        err.value_type_id     = v.tag;
        err.requested_type_id = meta::type_id<T>();
        return T();
    }
}

template <typename T>
bool Query<T>::get(Value const& v) {
    using NoConst   = std::remove_const_t<std::remove_reference_t<T>>;
    using NoPointer = std::remove_const_t<std::remove_pointer_t<NoConst>>;

    if constexpr (std::is_reference<T>::value) {
        return Query<NoConst*>::get(v);
    } else {
        if (v.tag == meta::type_id<NoConst>()) {
            return true;
        }
        if (v.tag == meta::type_id<NoConst*>()) {
            return true;
        }
        if constexpr (std::is_pointer_v<NoConst>) {
            return v.tag == meta::type_id<NoPointer>();
        }
        return false;
    }
}

//
// This is mostly not necessary excpet from Function
// which does not support the sizeof
//

#define GETTER(type, name)                                                         \
    template <>                                                                    \
    struct Getter<type> {                                                          \
        static type get(Value& v, GetterError& err) {                              \
            err.failed = v.tag != meta::type_id<type>();                           \
            return v.value.name;                                                   \
        };                                                                         \
        static type get(Value const& v, GetterError& err) {                        \
            err.failed = v.tag != meta::type_id<type>();                           \
            return v.value.name;                                                   \
        };                                                                         \
    };                                                                             \
    template <>                                                                    \
    struct Query<type> {                                                           \
        static bool get(Value const& v) { return v.tag == meta::type_id<type>(); } \
    };

GETTER(Function, fun)
// GETTER(bool, i1)

// KIWI_VALUE_TYPES(GETTER)
#undef GETTER

//
// ostream
//

std::ostream& operator<<(std::ostream& os, Value const& v);

//
// Automatic Function Wrapper
//

template <typename Sig>
struct Interop;

template <typename R, typename... Args>
struct Interop<R(Args...)> {
    using NativeArgs       = std::tuple<Args...>;
    using FunctionType     = R (*)(Args...);
    using ScriptValue      = Value;
    using ScriptArgs       = Array<Value>;
    using ScriptFunction_C = Function;

    template <typename T>
    static T to_native(Value& val) {
        return val.as<T>();
    }

    template <std::size_t... Indices>
    static R
    call_function(FunctionType func, void* mem, ScriptArgs& args, std::index_sequence<Indices...>) {
        return func(to_native<Args>(args[Indices])...);
    }

    template <FunctionType func>
    static ScriptValue wrapper(void* mem, ScriptArgs& args) {  //
        return call_function(func, mem, args, std::make_index_sequence<sizeof...(Args)>{});
    };
};

template <typename R, typename... Args>
struct Interop<R (*)(Args...)> {
    using NativeArgs       = std::tuple<Args...>;
    using FunctionType     = R (*)(Args...);
    using ScriptValue      = Value;
    using ScriptArgs       = Array<Value>;
    using ScriptFunction_C = Function;

    template <FunctionType func>
    static auto constexpr wrapper = Interop<R(Args...)>::template wrapper<func>;
};

template <typename R, typename O, typename... Args>
struct Interop<R (O::*)(Args...)> {
    using NativeArgs       = std::tuple<O*, Args...>;
    using FunctionType     = R (O::*)(Args...);
    using ScriptValue      = Value;
    using ScriptArgs       = Array<Value>;
    using ScriptFunction_C = Function;

    template <typename T>
    static T to_native(Value& val) {
        return val.as<T>();
    }

    template <std::size_t... Indices>
    static R
    call_method(FunctionType func, void* mem, ScriptArgs& args, std::index_sequence<Indices...>) {
        O* self = args[0].as<O*>();
        return (self->*func)(to_native<Args>(args[Indices + 1])...);
    }

    template <FunctionType method>
    static R freemethod(O* self, Args... args) {
        return (self->*method)(args...);
    }

    template <FunctionType func>
    static ScriptValue wrapper(void* mem, ScriptArgs& args) {  //
        return call_method(func, mem, args, std::make_index_sequence<sizeof...(Args)>{});
    };
};

template <typename R, typename O, typename... Args>
struct Interop<R (O::*)(Args...) const> {
    using NativeArgs       = std::tuple<O*, Args...>;
    using FunctionType     = R (O::*)(Args...) const;
    using ScriptValue      = Value;
    using ScriptArgs       = Array<Value>;
    using ScriptFunction_C = Function;

    template <typename T>
    static T to_native(Value& val) {
        return val.as<T>();
    }

    template <std::size_t... Indices>
    static R
    call_method(FunctionType func, void* mem, ScriptArgs& args, std::index_sequence<Indices...>) {
        O* self = args[0].as<O*>();

        return (self->*func)(to_native<Args>(args[Indices + 1])...);
    }

    template <FunctionType method>
    static R freemethod(O* self, Args... args) {
        return (self->*method)(args...);
    }

    template <FunctionType func>
    static ScriptValue wrapper(void* mem, ScriptArgs& args) {  //
        return call_method(func, mem, args, std::make_index_sequence<sizeof...(Args)>{});
    };
};

#define KIWI_WRAP(funfun) Function(Interop<decltype(&funfun)>::wrapper<(&funfun)>)

// Specialization for C object that have a custom free
using FreeFun = void (*)(void*);

template <typename T, FreeFun free_fun = std::free>
struct _destructor {
    static void free(Value& v) {
        if (v.value.obj == nullptr) {
            return;
        }

        // call the destructor
        ((T*)(v.value.obj))->~T();

        free_fun(v.value.obj);

        // NOTE: this only nullify current value so other copy of this value
        // might still think the value is valid
        // one thing we can do is allocate the memory using a pool.
        // on free the memory returns to the pool and it is marked as invalid
        // copied value will be able to check for the mark until the memory is reused
        // then same issue would be still be possible
        v.value.obj = nullptr;  // just in case
    }
};

template <FreeFun free_fun>
struct _custom_free {
    static void free(Value& v) {
        free_fun(v.value.obj);

        // NOTE: this only nullify current value so other copy of this value
        // might still think the value is valid
        // one thing we can do is allocate the memory using a pool.
        // on free the memory returns to the pool and it is marked as invalid
        // copied value will be able to check for the mark until the memory is reused
        // then same issue would be still be possible
        v.value.obj = nullptr;  // just in case
    }
};

template <typename T>
struct _copy {
    static Value copy(Value const& v) {
        auto [cpy, _] = make_value<T>(v.as<T const&>());
        return cpy;
    }
};

template <typename T>
struct _ref {
    static Value ref(Value& v) {
        auto [ref, _] = make_value<T*>(v.as<T*>());
        return ref;
    }
};

template <typename T>
struct _hash {
    static std::size_t hash(Value const& v) { return std::hash<T>()(v.as<T const&>()); }
};

template <typename T>
struct _printer {
    static void print(std::ostream& out, Value const& v) { out << v.as<T const&>(); }
};

//
// Custom object wrapper
//
//  Some lib take care of the allocation for us
//  so this would  not work, and we would have to allocate 2 twice (once from
//  the lib & another for us)
//

template <typename T, typename... Args>
Tuple<Value, ValueDeleter> _new_object(int _typeid, Args... args) {
    // up to the user to free it correctly
    void* memory = malloc(sizeof(T));
    new (memory) T(args...);
    return std::make_tuple(Value(_typeid, memory), _destructor<T>::free);
}

inline void noop_destructor(Value& v) {}

template <typename T, typename... Args>
Tuple<Value, ValueDeleter> _new_value(int _typeid, Args... args) {
    // The value cannot have a destructor here
    static_assert(Value::is_small<T>());
    Value value;
    new (&value.value) T(args...);
    value.tag = _typeid;
    return std::make_tuple(value, noop_destructor);
}

// This version allows users to specify a different type id
// so some dynamic DS could be used multiple time with a different typeid
template <typename T, typename... Args>
Tuple<Value, ValueDeleter> _make_value(int _typeid, Args... args) {
    if constexpr (Value::is_small<T>()) {
        return _new_value<T>(_typeid, args...);
    }
    return _new_object<T>(_typeid, args...);
}

template <typename T, FreeFun fun = nullptr>
ValueDeleter value_deleter() {
    if constexpr (fun == nullptr) {
        if constexpr (Value::is_small<T>()) {
            return noop_destructor;
        }
        // C++ destructor + delete
        return _destructor<T>::free;
    }
    // C free function (no destructor)
    return _custom_free<fun>::free;
}

template <typename T>
ValueCopier value_copier() {
    // Call the C++ copy constructor
    return _copy<T>::copy;
}

bool register_metadata(int          type_id,
                       const char*  name,
                       ValueDeleter deleter,
                       ValueCopier  copier,
                       ValuePrinter printer,
                       ValueHash    hasher,
                       ValueRef     refmaker);

template <typename S, typename T>
class is_streamable {
    template <typename SS, typename TT>
    static auto test(int) -> decltype(std::declval<SS&>() << std::declval<TT>(), std::true_type());

    template <typename, typename>
    static auto test(...) -> std::false_type;

    public:
    static const bool value = decltype(test<S, T>(0))::value;
};

// Short cut
template <typename T, typename... Args>
Tuple<Value, ValueDeleter> make_value(Args... args) {
    return _make_value<T>(meta::type_id<T>(), args...);
}
template <typename T, typename... Args>
Tuple<Value, ValueDeleter> new_value(Args... args) {
    return _new_value<T>(meta::type_id<T>(), args...);
}
template <typename T, typename... Args>
Tuple<Value, ValueDeleter> new_object(Args... args) {
    return _new_object<T>(meta::type_id<T>(), args...);
}

template <typename T, FreeFun fun, typename... Args>
Tuple<Value, ValueDeleter> from_pointer(T* raw) {
    Value v;
    v.tag = meta::type_id<T*>();
    new (v.pointer<T*>()) T*(raw);
    return std::make_tuple(v, _custom_free<fun>::free);
}

//
// Invoke a script function with native values or script values
//
// ctx is used when calling Script function where the VM is needed
//
template <typename... Args>
Value invoke(void* ctx, Value fun, Args... args) {
    Array<Value> value_args = {Value(args)...};
    return fun.as<Function>()(ctx, value_args);
}

// template <typename... Args>
// Value call(Value fun, Args... values) {
//     return fun.as<Function>()(nullptr, Array<Value>{values...});
// }

Value binary_invoke(void* ctx, Value fun, Value a, Value b);

Value unary_invoke(void* ctx, Value fun, Value a);

//
// Auto register STL operators if defined
//
template <typename T>
bool _register_value(ValuePrinter printer = nullptr) {

    ValueHash hasher = nullptr;
    if constexpr (std::is_default_constructible<std::hash<T>>::value) {
        hasher = _hash<T>::hash;
    }

    if constexpr (is_streamable<std::ostream, T>::value) {
        if (printer == nullptr) {
            printer = _printer<T>::print;
        }
    }

    ValueCopier copy = nullptr;
    if constexpr (std::is_copy_constructible_v<T>) {
        copy = value_copier<T>();
    }

    ValueRef ref = nullptr;
    // if constexpr (std::is_trivially_copyable<T>::value) {
    //     ref = _ref<T>::ref();
    // }

    register_metadata(
        meta::type_id<T>(), typeid(T).name(), value_deleter<T>(), copy, printer, hasher, ref);

    return true;
}

template <typename T>
void register_value(ValuePrinter printer = nullptr) {
    static bool _ = _register_value<T>(printer);
}

}  // namespace lython

namespace std {

template <>
struct hash<lython::Value> {
    std::size_t operator()(lython::Value const& k) const noexcept { return k.hash(); }
};

}  // namespace std