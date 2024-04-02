
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

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
using Function = Value (*)(Array<Value> const&);

struct Struct {
    void* ptr;

    // we could save the deleter here and remove ManagedVariable
    // at the price of a bigger overhead
    // void(*deleter)(void*) = nullptr
};

struct None {};

#define TYPES(X)     \
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
// some bit of the value as tag bit.
//

struct Value {
    union Holder {
#define ATTR(type, name) type name;
        TYPES(ATTR)
#undef ATTR

        void* obj;
    };

    Holder value;
    uint32 tag;

    Value(): tag(type_id<None>()) {}

#define CTOR(type, name) \
    Value(type name): tag(type_id<type>()) { value.name = name; }

    TYPES(CTOR)
#undef CTOR

    Value(int tag, void* ptr): tag(tag) { value.obj = ptr; }


    bool is_type(int obj_type_id) const {
        return obj_type_id == tag;
    }

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
        return sizeof(T) > sizeof(Value::Holder);
    }

    template <typename T>
    void* get_storage() {
        if (is_object()) {
            // use itself as storage
            if (!is_allocated<T>()) {
                return this;
            }
            // object was too big and had to be allocated
            return this->value.obj;
        }
    }
};

template <typename T>
T Getter<T>::get(Value& v) {
    using Underlying = std::remove_pointer_t<T>;
    static T def     = nullptr;

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
    if (!Value::is_allocated<Underlying>()) {
        Value value;
        new (&value) Underlying(args...);
        value.tag = _typeid;
        return std::make_tuple(value, [](Value) {});
    }

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
        // one thing we can do is allocate the memory using a pool
        // on free the memory returns to the pool and it is marked as invalid
        // copied value will be able to check for the mark
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
    Point(float x, float y): x(x), y(y) {}

    ~Point() { std::cout << "Destructor called" << "\n"; }

    float x, y;

    float distance() const { return sqrt(x * x + y * y); }
};

// This struct is too big and it will be allocated on the heap
struct Rectangle {
    Rectangle(Point p, Point s): p(p), s(s)
    {}

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
};


// 

//
// Invoke a script function with native values or script values
//
template <typename... Args>
Value invoke(Value fun, Args... args) {
    Array<Value> value_args = {Value(args)...};
    return fun.as<Function>()(value_args);
}

int main() {
    std::cout << " Value size: " << sizeof(Value) << "\n";
    std::cout << "Holder size: " << sizeof(Value::Holder) << "\n";

    Value a(10);
    Value b(11);

    int r = a.as<int>() + b.as<int>();

    // Manual Wrap
    //
    // Wrap a native function to be called with script values

    Value distance([](Array<Value> const& args) -> Value {
        Value a = args[0];
        return Value(a.as<Point*>()->distance());
    });

    // Auto wrap
    {
        // manual free
        auto [value, deleter] = make_value<Point>(3.0, 4.0);

        std::cout << "invoke: " << invoke(distance, value) << "\n";

        // free_value(p, destructor<Point>);
        // OR
        deleter(value);
    }

    {
        auto [value, deleter] = make_value<Rectangle>(Point(3.0, 4.0), Point(3.0, 4.0));

        deleter(value);
    }

    // Object created at runtime;
    {
        // create a new type_id
        int my_type_id = _new_type();

        auto [value, deleter] = _make_value<ScriptObject>(my_type_id);

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