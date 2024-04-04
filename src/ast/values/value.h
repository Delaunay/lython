#pragma once

#include "utilities/metadata.h"

#define KIWI_SVO 1

namespace lython {

struct Value;

struct GetterError {
    bool failed = false;
};

template<typename T>
struct Maybe {
    T* value = nullptr;
};

template <typename T>
struct Getter {
    static T get(Value& v, GetterError& err);
    static const T get(const Value& v, GetterError& err);
};

using ValueDeleter = std::function<void(Value)>;

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
        KIWI_VALUE_TYPES(ATTR)
#undef ATTR

        void* obj;
    };

    // The VM might not need the type tag
    // once sema is passed we should be able to guarantee
    // the oprations are ok
    std::conditional_t<true, uint32, void> tag;
    Holder value;

    Value(): tag(meta::type_id<_None>()) {}

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
        return is_type(type_id<T>());
    }

    template<typename T>
    bool operator==(T const& val) const {
        if (tag == meta::type_id<T>()) {
            return as<T>() == val;
        }
        return false;
    }

    template<typename T>
    bool operator!=(T const& val) const {
        return !((*this) == val);
    }

    bool operator==(Value const& val) const;

    bool operator!=(Value const& val) const {
        return !((*this) == val);
    }

    template <typename T>
    T as() {
        GetterError err;
        return Getter<T>::get(*this, err);
    }

    template <typename T>
    T as() const {
        GetterError err;
        return Getter<T>::get(*this, err);
    }

    bool is_object() const { return tag >= int(meta::ValueTypes::Max); } 

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
        }
        else {
            // The data is stored in dynamically allocated memory
            return reinterpret_cast<T*>(value.obj); 
        }
    }

    template <typename T>
    T const* pointer() const {
        if (!Value::is_allocated<T>()) {
            return reinterpret_cast<T const*>(&value);
        }
        return reinterpret_cast<T const*>(value.obj); 
    }
};

//
// Getter
//
template <typename T>
T Getter<T>::get(Value& v, GetterError& err) {
    using NoConst   = std::remove_const_t<std::remove_reference_t<T>>;
    using NoPointer = std::remove_const_t<std::remove_pointer_t<NoConst>>;

    if (v.tag == meta::type_id<NoConst>()) {
        T* ptr = v.pointer<NoConst>();
        return *ptr;
    }
    
    if constexpr (std::is_pointer_v<NoConst>){
        if (v.tag == meta::type_id<NoPointer>()) {
            NoPointer* ptr = v.pointer<NoPointer>();
            return ptr;
        }
    }

    err.failed = true;
    return T();
}

template <typename T>
T const Getter<T>::get(Value const& v, GetterError& err) {
    using NoConst   = std::remove_const_t<std::remove_reference_t<T>>;
    using NoPointer = std::remove_const_t<std::remove_pointer_t<NoConst>>;

    // We want to fetch the value 
    if (v.tag == meta::type_id<NoConst>()) {
        NoConst const* ptr = v.pointer<NoConst>();
        return *ptr;
    }

    if constexpr (std::is_pointer_v<NoConst>){
        if (v.tag == meta::type_id<NoPointer>()) {
            NoPointer const* ptr = v.pointer<NoPointer>();
            return ptr;
        }
    }

    err.failed = true;
    return T();
}

//
// This is mostly not necessary excpet from Function
// which does not support the sizeof
//

#define GETTER(type, name)                               \
    template <>                                          \
    struct Getter<type> {                                \
        static type get(Value& v, GetterError& err) {    \
            err.failed = v.tag != meta::type_id<type>(); \
            return v.value.name;                         \
        };                                               \
        static type get(Value const& v, GetterError& err) { \
            err.failed = v.tag != meta::type_id<type>(); \
            return v.value.name;                         \
        };                                               \
    };

KIWI_VALUE_TYPES(GETTER)
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

#define KIWI_WRAP(fun) Function(Interop<decltype(&(fun))>::template wrapper<&(fun)>)

// ---
void free_value(Value val, void (*deleter)(void*) = nullptr);

template <typename T>
void destructor(void* ptr) {
    ((T*)(ptr))->~T();
    free(ptr);
}

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
    auto deleter = [](Value val) { free_value(val, destructor<T>); };
    return std::make_tuple(Value(_typeid, memory), deleter);
}

template <typename T, typename... Args>
Tuple<Value, ValueDeleter> _new_value(int _typeid, Args... args) {
    static_assert(Value::is_small<T>());
    Value value;
    new (&value.value) T(args...);
    value.tag = _typeid;
    return std::make_tuple(value, [](Value v) {} );
}

// This version allows users to specify a different type id
// so some dynamic DS could be used multiple time with a different typeid
template <typename T, typename... Args>
Tuple<Value, ValueDeleter> _make_value(int _typeid, Args... args) 
{
    if constexpr (Value::is_small<T>()) {
        return _new_value<T>(_typeid, args...);
    }
    return _new_object<T>(_typeid, args...);
}

// Short cut
template <typename T, typename... Args>
Tuple<Value, std::function<void(Value)>> make_value(Args... args) {
    return _make_value<T>(meta::type_id<T>(), args...);
}
template <typename T, typename... Args>
Tuple<Value, std::function<void(Value)>> new_value(Args... args) {
    return _new_value<T>(meta::type_id<T>(), args...);
}
template <typename T, typename... Args>
Tuple<Value, std::function<void(Value)>> new_object(Args... args) {
    return _new_object<T>(meta::type_id<T>(), args...);
}


template <typename T, typename... Args>
Tuple<Value, std::function<void(Value)>> make_value(T* raw, void (*custom_free)(void*)) {

    auto deleter = [custom_free](Value val) { free_value(val, custom_free); };

    return std::make_tuple(Value(meta::type_id<T>(), raw), deleter);
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

Value binary_invoke(void* ctx, Value fun, Value a, Value b);

Value unary_invoke(void* ctx, Value fun, Value a);

// template <Value... Args>
// Value invoke(void* ctx, Value fun, Value... args) {
//     Array<Value> value_args = {args...};
//     return fun.as<Function>()(ctx, value_args);
// }

}  // namespace lython