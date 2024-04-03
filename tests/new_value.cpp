
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

    // Convert script value to native
    static void from_script(NativeArgs& arguments, ScriptArgs const& args) {
        packargs<std::tuple_size_v<std::remove_reference_t<NativeArgs>>>(arguments, args);
    }

    // Convert script arguments to native arguments
    template <int N, typename... Types>
    static int packargs(std::tuple<Types...>& destination, ScriptArgs const& args) {
        using TupleSize = typename std::tuple_size<std::remove_reference_t<std::tuple<Types...>>>;

        if constexpr (N > 0) {
            using ElemT =
                typename std::tuple_element<TupleSize::value - N, std::tuple<Types...>>::type;
            ScriptValue val = args[TupleSize::value - N];

            if (val.is_object()) {
                if constexpr (std::is_pointer<ElemT>::value) {
                    ElemT& dest = std::get<TupleSize::value - N>(destination);
                    dest        = val.as<ElemT>();
                }

                else {
                    ElemT& dest = std::get<TupleSize::value - N>(destination);
                    dest        = val.as<ElemT*>();
                }
            }

            packargs<N - 1>(destination, args);
        }

        return 0;
    }

    template <typename T>
    static T to_native(Value& val) {
        return val.as<T>();
    }

    template <std::size_t... Indices>
    static R
    call_function(FunctionType func, void* mem, ScriptArgs& args, std::index_sequence<Indices...>) {
        // Convert each script argument to native type and call the function with them
        return func(to_native<Args>(args[Indices])...);
    }

    template <FunctionType func>
    static ScriptValue wrapper(void* mem, ScriptArgs& args) {  //
        // Unpack args to the correct Type into Packed
        // Does not work on MSVC

        // NativeArgs arguments;
        // from_script(arguments, args);
        // // this does not work with mscv
        // Value v = std::apply(  //
        //     func,              //
        //     arguments          //
        // );

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

    // template <FunctionType func>
    // static auto constexpr wrapper =
    //     Interop<R(O*, Args...)>::template wrapper<&Interop::freemethod<func>>;
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
}

//
// Custom object wrapper
//
//  Some lib take care of the allocation for us
//  so this would  not work, and we would have to allocate 2 twice (once from
//  the lib & another for us)
//
template <typename T, typename... Args>
Tuple<Value, std::function<void(Value)>> _make_value(int _typeid, Args... args) {
    using Underlying = std::remove_pointer_t<T>;

//
// When a struct can fit in the union, we put it in
// Then the deleter becomes a noop
//
#if KIWI_SVO
    if (!Value::is_allocated<Underlying>()) {
        Value value;
        // C++ does not guarantee that there is no padding in FRONT but C does
        // probably for vtables
        void* mem = reinterpret_cast<uint8*>(&value) + offsetof(Value, value);
        new (mem) Underlying(args...);
        value.tag = _typeid;

        // NOTE: trivially copyable means there is no specific destructor so we could have a noop
        // here
        return std::make_tuple(value, [](Value v) { destructor<T>(v.get_storage<T>()); });
    }
#endif

    // up to the user to free it correctly
    void* memory = malloc(sizeof(Underlying));
    ;
    new (memory) Underlying(args...);

    auto deleter = [](Value val) { free_value(val, destructor<T>); };

    return std::make_tuple(Value(_typeid, memory), deleter);
}

template <typename T, typename... Args>
Tuple<Value, std::function<void(Value)>> make_value(Args... args) {
    using Underlying = std::remove_pointer_t<T>;

    return _make_value<T>(type_id<Underlying>(), args...);
}

template <typename T, typename... Args>
Tuple<Value, std::function<void(Value)>> make_value(T* raw, void (*custom_free)(void*)) {

    auto deleter = [custom_free](Value val) { free_value(val, custom_free); };

    return std::make_tuple(Value(type_id<T>(), raw), deleter);
}

void free_value(Value val, void (*deleter)(void*)) {
    // we don't know the type here
    // we have to tag the value to know no memory was allocated
    // if (sizeof(T) <= sizeof(Value::Holder)) {
    //     return;
    // }

    if (val.is_object()) {
        if (deleter != nullptr) {
            deleter(val.value.obj);
        }

        // NOTE: this only nullify current value so other copy of this value
        // might still think the value is valid
        // one thing we can do is allocate the memory using a pool.
        // on free the memory returns to the pool and it is marked as invalid
        // copied value will be able to check for the mark until the memory is reused
        // then same issue would be still be possible
        val.value.obj = nullptr;  // just in case
    }
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

// This struct is small enough and will be stored on the stack
struct Point {
    Point(float xx, float yy): x(xx), y(yy) {}

    float x = 0;
    float y = 0;

    float distance() const { return sqrt(x * x + y * y); }

    float distance2() { return sqrt(x * x + y * y); }
};

float freefun_distance(Point const* p) { return sqrt(p->x * p->x + p->y * p->y); }

// This struct is too big and it will be allocated on the heap
struct Rectangle {
    Rectangle(Point p, Point s): p(p), s(s) {}

    Point p;
    Point s;
};

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

// Trivial
template <typename T, int N>
struct MyArray {
    T values[N];
};

int main() {

#define SHOW_SIZE(type, name) std::cout << #name << ": " << sizeof(type) << "\n";
    TYPES(SHOW_SIZE)
#undef GETTER

    std::cout << "      Holder size: " << sizeof(Value::Holder) << "\n";
    std::cout << "       Value size: Min: " << sizeof(Value::Holder) + sizeof(uint32)
              << " Actual: " << sizeof(Value) << "\n";

    std::cout << "ScriptObject size: " << sizeof(ScriptObject) << "\n";
    std::cout << "      String size: " << sizeof(String) << "\n";
    std::cout << "       Array size: " << sizeof(Array<int>) << "\n";
    std::cout << "       Rect size: " << sizeof(Rectangle) << "\n";

    {
        // Not trivially copyable because of optim
        // probably actually trivially copyable for POD
        using TT = Tuple<int, int>;
        std::cout << "       Tuple size: " << sizeof(TT) << " "
                  << std::is_trivially_copyable<TT>::value << "\n";
    }

    {
        using TT = MyArray<int, 2>;
        std::cout << "       MyArray size: " << sizeof(TT) << " "
                  << std::is_trivially_copyable<TT>::value << "\n";
    }

    Value a(10);
    Value b(11);

    int r = a.as<int>() + b.as<int>();

    // Manual Wrap
    //
    // Wrap a native function to be called with script values

    Value distance([](void*, Array<Value>& args) -> Value {
        Value a = args[0];
        return Value(a.as<Point*>()->distance());
    });

    // Value wrapped = Interop<float(Point const*)>::template wrapper<&freefun_distance>;
    // Value wrapped = Interop<decltype(&freefun_distance)>::template wrapper<&freefun_distance>;
    Value wrapped      = KIWI_WRAP(freefun_distance);
    Value const_method = KIWI_WRAP(Point::distance);
    Value method       = KIWI_WRAP(Point::distance2);

    // Auto wrap
    {
        std::cout << "Point is_trivially_copyable: " << std::is_trivially_copyable<Point>::value
                  << "\n";
        // manual free
        auto [value, deleter] = make_value<Point>(3.0, 4.0);
        Value copy            = value;

        std::cout << "x == 3: " << copy.as<Point*>()->x << "\n";
        std::cout << "invoke: " << invoke(nullptr, distance, value) << "\n";
        std::cout << "invoke: " << invoke(nullptr, wrapped, value) << "\n";
        std::cout << "invoke: " << invoke(nullptr, method, value) << "\n";
        std::cout << "invoke: " << invoke(nullptr, const_method, value) << "\n";

        value.as<Point*>()->x = 2;
        std::cout << "x == 2: " << value.as<Point*>()->x << "\n";
        std::cout << "x == 3: " << copy.as<Point*>()->x << "\n";

        // free_value(p, destructor<Point>);
        // OR
        deleter(value);
    }

    {
        std::cout << "Rectangle is_trivially_copyable: "
                  << std::is_trivially_copyable<Rectangle>::value << "\n";

        auto [value, deleter] = make_value<Rectangle>(Point(3.0, 4.0), Point(3.0, 4.0));

        Value copy = value;

        std::cout << "x == 3 " << copy.as<Rectangle*>()->p.x << "\n";
        copy.as<Rectangle*>()->p.x = 2;
        std::cout << "x == 2 " << copy.as<Rectangle*>()->p.x << "\n";

        // Object did not really get copied because it is allocated
        std::cout << "x == 2 " << value.as<Rectangle*>()->p.x << "\n";

        deleter(value);
    }

    // Object created at runtime;
    {
        std::cout << "ScriptObject is_trivially_copyable: "
                  << std::is_trivially_copyable<ScriptObject>::value << "\n";

        // create a new type_id
        int my_type_id = _new_type();

        auto [value, deleter] = _make_value<ScriptObject>(my_type_id);

        Value v1 = value;
        Value v2 = value;

        v2.as<ScriptObject*>()->attributes.push_back(std::make_tuple("String", Value(0)));

        std::cout << v2.as<ScriptObject*>()->attributes.size() << std::endl;

        assert(value.is_type(my_type_id));
    }

#if 0
  // C Wrap
  {
    SDL_Window* window;
    auto [value, deleter] = make_value<SDL_Window>(window, SDL_DestroyWindow);

    deleter(value);
  }
#endif

    {
        // auto free
        ManagedValue pp = make_managed_value<Point>(1, 2);

        pp.destroy();
    }

    std::cout << "size: " << sizeof(Value) << "\n";
    return (a.as<int>() + r) * 0;
}