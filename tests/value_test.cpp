
#include "utilities/metadata.h"
#include "ast/values/value.h"
#include "utilities/printing.h"

#include <algorithm>
#include <iostream>
#include <numeric>

#include <catch2/catch_all.hpp>

using namespace lython;

#define APPROX(x) Catch::Approx(x)

TEST_CASE("Value_Base") {
    Value a(1);
    Value b(2);
    Value c(1);

    REQUIRE(a == 1);
    REQUIRE(b != 1);
    REQUIRE(a != b);
    REQUIRE(a == c);
    REQUIRE(a != 1.0f);
}

// This struct is small enough and will be stored on the stack
struct Point2D {
    Point2D() {}

    Point2D(float xx, float yy): x(xx), y(yy) {}

    float x = 0;
    float y = 0;

    float distance() const { return sqrt(x * x + y * y); }

    float distance2() { return sqrt(x * x + y * y); }

    bool operator== (Point2D const& other) const {
        return x == other.x && y == other.y;
    }
};

float freefun_distance(Point2D const* p) { return sqrt(p->x * p->x + p->y * p->y); }


template <>
struct lython::meta::ReflectionTrait<Point2D> {
    static int register_members() {
        #define register_prop(Type, property)                               \
            lython::meta::register_property<&Type::property>(#property)    

        register_prop(Point2D, x);
        
        lython::meta::register_property<&Point2D::x>("x");
        lython::meta::register_property<&Point2D::y>("y");


        //lython::meta::new_member<Point2D, float>("x", offsetof(Point2D, x));
        //lython::meta::new_member<Point2D, float>("y");
        // lython::meta::new_method("add", &Pnt::add);
        // lython::meta::new_method("sum", &Pnt::sum);

        return 1;
    }
};


TEST_CASE("Value_SVO_Function Wrapping") {
    Value distance([](void*, Array<Value>& args) -> Value {
        Value a = args[0];
        return Value(a.pointer<Point2D>()->distance());
    });

    auto  _            = &Point2D::distance2;
#if 0
    Value wrapped      = KIWI_WRAP(freefun_distance);
    Value method       = KIWI_WRAP(Point2D::distance2);
    Value const_method = KIWI_WRAP(Point2D::distance);
#else
    Value wrapped      = kiwi_function<&freefun_distance>();
    Value method       = kiwi_function<&Point2D::distance2>();
    Value const_method = kiwi_function<&Point2D::distance>();
#endif

    auto value = make_value<Point2D>(3.0f, 4.0f);
    Value copy            = value;

    REQUIRE(copy.is_valid<Point2D*>() == true);
    REQUIRE(copy.is_valid<Point2D const*>() == true);

    REQUIRE(copy.pointer<Point2D>() != value.pointer<Point2D>());
    REQUIRE(Value::has_error() == false);

    // Taking the address of a reference is UB
    REQUIRE(copy.is_valid<Point2D&>() == true);
    REQUIRE(&copy.as<Point2D&>() == copy.pointer<Point2D>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.is_valid<Point2D const&>() == true);
    REQUIRE(&copy.as<Point2D const&>() == copy.pointer<Point2D>());
    REQUIRE(Value::has_error() == false);

    // Retrieve a copy of the value
    REQUIRE(copy.is_valid<Point2D>() == true);
    REQUIRE(copy.as<Point2D>().x == 3.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.as<Point2D>().y == 4.0f);
    REQUIRE(Value::has_error() == false);

    // Retrieve a pointer of the value
    REQUIRE(copy.as<Point2D*>()->y == 4.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.as<Point2D const*>()->y == 4.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, distance, value).as<float>() == 5.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped, value).as<float>() == 5.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, method, value).as<float>() == 5.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, const_method, value).as<float>() == 5.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, distance, copy).as<float>() == 5.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped, copy).as<float>() == 5.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, method, copy).as<float>() == 5.0f);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, const_method, copy).as<float>() == 5.0f);
    REQUIRE(Value::has_error() == false);

    // REQUIRE(deleter == noop_destructor);
}

// This struct is too big and it will be allocated on the heap
struct Rectangle {
    Rectangle() {}

    Rectangle(Point2D p, Point2D s): p(p), s(s) {}

    Point2D p;
    Point2D s;

    float perimeter() const { return (s.x + s.y) * 2.0f; }

    float perimeter2() { return (s.x + s.y) * 2.0f; }

    bool operator== (Rectangle const& op) const {
        return p == op.p && s == op.s;
    }
};

template <>
struct lython::meta::ReflectionTrait<Rectangle> {
    static int register_members() {
        lython::meta::new_member<Rectangle, Point2D>("p");
        lython::meta::new_member<Rectangle, Point2D>("s");
        // lython::meta::new_method("add", &Pnt::add);
        // lython::meta::new_method("sum", &Pnt::sum);

        return 1;
    }
};


float freefun_perimeter_cst(Rectangle const* p) { return (p->s.x + p->s.y) * 2.0f; }
float freefun_perimeter_cst_ref(Rectangle const& p) { return (p.s.x + p.s.y) * 2.0f; }
float freefun_perimeter(Rectangle* p) { return (p->s.x + p->s.y) * 2.0f; }
float freefun_perimeter_ref(Rectangle& p) { return (p.s.x + p.s.y) * 2.0f; }
float freefun_perimeter_cpy(Rectangle p) { return (p.s.x + p.s.y) * 2.0f; }

TEST_CASE("Value_NOSVO_Function Wrapping") {
    Value distance([](void*, Array<Value>& args) -> Value {
        Value a = args[0];
        return Value(a.as<Rectangle>().perimeter());
    });

    Value wrapped         = KIWI_WRAP(freefun_perimeter);
    Value wrapped_ref     = KIWI_WRAP(freefun_perimeter_ref);
    Value wrapped_cst     = KIWI_WRAP(freefun_perimeter_cst);
    Value wrapped_cst_ref = KIWI_WRAP(freefun_perimeter_cst_ref);
    Value wrapped_cpy     = KIWI_WRAP(freefun_perimeter_cpy);
    Value method          = KIWI_WRAP(Rectangle::perimeter2);
    Value const_method    = KIWI_WRAP(Rectangle::perimeter);

    auto value = make_value<Rectangle>(Point2D(3.0f, 4.0f), Point2D(3.0f, 4.0f));
    Value copy            = value;

#define REF(T, val) ((T&)(*val.as<T*>()))

    REQUIRE(copy.is_valid<Rectangle>() == true);
    REQUIRE(copy.pointer<Rectangle>() == value.pointer<Rectangle>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.is_valid<Rectangle*>() == true);
    REQUIRE(copy.as<Rectangle*>() == value.pointer<Rectangle>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.is_valid<Rectangle const*>() == true);
    REQUIRE(copy.as<Rectangle const*>() == value.pointer<Rectangle>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.is_valid<Rectangle&>() == true);
    // Taking the address of a reference is UB
    REQUIRE(&copy.as<Rectangle&>() == value.pointer<Rectangle>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.is_valid<Rectangle const&>() == true);
    REQUIRE(&copy.as<Rectangle const&>() == value.pointer<Rectangle>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, distance, value).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped, value).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, method, value).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, const_method, value).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, distance, copy).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped, copy).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped_cpy, copy).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped_ref, copy).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped_cst, copy).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped_cst_ref, copy).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, method, copy).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, const_method, copy).as<float>() == 14.0);
    REQUIRE(Value::has_error() == false);

    value.destroy();
    //deleter(nullptr, value);
}

TEST_CASE("Value_Reference") {

    int i = 4;

    auto value = make_value<int*>(&i);

    int*& ptrref = value.ref<int*>();

    // COOL
    REQUIRE(value.is_valid<int>() == true);
    REQUIRE(value.as<int>() == 4);
    REQUIRE(Value::has_error() == false);

    // Useless for that type
    REQUIRE(value.is_valid<int const>() == true);
    REQUIRE(value.as<int const>() == 4);
    REQUIRE(Value::has_error() == false);
    // ---

    REQUIRE(value.is_valid<int*&>() == true);
    REQUIRE(value.as<int*&>() == (&i));
    REQUIRE(Value::has_error() == false);

    REQUIRE(value.is_valid<int* const&>() == true);
    REQUIRE(value.as<int* const&>() == (&i));
    REQUIRE(Value::has_error() == false);

    REQUIRE(value.is_valid<int*>() == true);
    REQUIRE((value.as<int*>()) == (&i));
    REQUIRE(Value::has_error() == false);

    REQUIRE(value.is_valid<int**>() == true);
    REQUIRE((*value.as<int**>()) == (&i));
    REQUIRE(Value::has_error() == false);

    REQUIRE((**value.as<int**>()) == 4);
    REQUIRE(Value::has_error() == false);

    i = 5;
    REQUIRE((**value.as<int**>()) == 5);
    REQUIRE(Value::has_error() == false);

    // REQUIRE(deleter == noop_destructor);
}

struct cstruct {
    float a;
};

cstruct* new_cstruct() {
    auto ptr = (cstruct*)malloc(sizeof(cstruct));
    ptr->a   = 2.0f;
    return ptr;
}

void free_cstruct(void* data) { free(data); }

TEST_CASE("Value_C_Object") {

    cstruct* ptr = new_cstruct();

    auto value = from_pointer<cstruct, free_cstruct>(ptr);

    REQUIRE(value.is_valid<cstruct*>() == true);
    REQUIRE(value.as<cstruct*>() == ptr);
    REQUIRE(Value::has_error() == false);

    REQUIRE(value.as<cstruct*>()->a == 2.0f);
    REQUIRE(Value::has_error() == false);

    value.destroy();
    value.destroy();
    // deleter(nullptr, value);Point2D
}

struct ScriptObjectTest {
    Array<Value> members;
};

// This advantage of this is that it groups memory allocation in one
// while if we used an Array it would allocate twice
// it would also fragment memory more as their would be 2 ptr jump
template <int N>
struct ScriptObjectFixed {
    Value members[N];
};

TEST_CASE("Value_Script_Check") {
    std::cout << "Holder: " << sizeof(Value::Holder) << "\n";
    std::cout << "Fixed1: " << sizeof(ScriptObjectFixed<1>) << "\n";
    std::cout << "Fixed2: " << sizeof(ScriptObjectFixed<2>) << "\n";
    std::cout << "Fixed3: " << sizeof(ScriptObjectFixed<3>) << "\n";
    std::cout << "Fixed4: " << sizeof(ScriptObjectFixed<4>) << "\n";
    std::cout << "   Dyn: " << sizeof(ScriptObjectTest) << "\n";
}

TEST_CASE("Value_ErrorHandling") {
    auto a = make_value<int>(1);

    REQUIRE(a.is_valid<float>() == false);
    REQUIRE(a.is_valid<int>() == true);

    a.as<float>();
    REQUIRE(Value::has_error() == true);
    Value::reset_error();

    a.as<float const>();
    REQUIRE(Value::has_error() == true);

    REQUIRE(Value::global_err.requested_type_id == meta::type_id<float const>());
    REQUIRE(Value::global_err.value_type_id == meta::type_id<int>());
    Value::reset_error();

    a.destroy();
    // deleter(nullptr, a);
}

TEST_CASE("Value_ErrorHandling_2") {

#define CASE(T, TT)                                                             \
    {                                                                           \
        std::cout << #T << " " << meta::type_id<T>() << "\n";                   \
        REQUIRE(v.is_valid<T>() == false);                                      \
        v.as<T>();                                                              \
        REQUIRE(Value::has_error() == true);                                    \
        REQUIRE(Value::global_err.requested_type_id == meta::type_id<TT>());    \
        REQUIRE(Value::global_err.value_type_id == meta::type_id<Rectangle>()); \
        Value::reset_error();                                                   \
    }

    {
#define TRY(X)              \
    X(int, int)             \
    X(int const, int const) \
    X(int const*, int const*)

        // X(int* const, int* const)
        // X(int const&, int*)
        // X(Rectangle*const*, Rectangle*const*)   \
            //

        auto vv = make_value<Rectangle>(Point2D(1, 1), Point2D(2, 2));
        Value const v      = vv;
        TRY(CASE)
        vv.destroy();
#undef TRY
    }
    {
#define TRY(X)                              \
    X(int, int)                             \
    X(int*, int*)                           \
    X(int&, int*)                           \
    X(int const, int const)                 \
    X(int const*, int const*)               \
    X(int const&, int*)                     \
    X(Rectangle* const*, Rectangle* const*) \
    X(int* const, int* const)

        auto vv = make_value<Rectangle>(Point2D(1, 1), Point2D(2, 2));
        Value v            = vv;
        TRY(CASE)
        vv.destroy();
#undef TRY
    }
#undef CASE

#define CASE(T, TT)                                           \
    {                                                         \
        std::cout << #T << " " << meta::type_id<T>() << "\n"; \
        REQUIRE(v.is_valid<T>() == true);                     \
        v.as<T>();                                            \
    }

    {
#define TRY(X)               \
    X(Rectangle, int)        \
    X(Rectangle const, int)  \
    X(Rectangle const*, int) \
    X(Rectangle const&, int)

        auto vv = make_value<Rectangle>(Point2D(1, 1), Point2D(2, 2));
        Value const v      = vv;
        TRY(CASE)
        vv.destroy();
#undef TRY
    }
    {
#define TRY(X)               \
    X(Rectangle, int)        \
    X(Rectangle*, int)       \
    X(Rectangle&, int)       \
    X(Rectangle const, int)  \
    X(Rectangle const*, int) \
    X(Rectangle const&, int) \
    X(Rectangle* const, int)

        auto vv = make_value<Rectangle>(Point2D(1, 1), Point2D(2, 2));
        Value v            = vv;
        TRY(CASE)
        vv.destroy();
#undef TRY
    }

#undef CASE
#undef TRY
}

// REQUIRE(Value::has_error() == false);                                    \
        // REQUIRE(Value::global_err.requested_type_id == meta::type_id<TT>());     \
        // REQUIRE(Value::global_err.value_type_id == meta::type_id<Rectangle>()); \
        // Value::reset_error();                                                   \


#define SUM(x) std::accumulate((x).begin(), (x).end(), 0.f)

float sum_array_const_ptr(Array<float> const* p) { return SUM(*p); }
float sum_array_const_ref(Array<float> const& p) { return SUM(p); }
float sum_array_ptr(Array<float>* p) { return SUM(*p); }
float sum_array_ref(Array<float>& p) { return SUM(p); }
float sum_array_cpy(Array<float> p) { return SUM(p); }

TEST_CASE("Value_Array Wrapping") {
    Value wrapped_cst_ptr = KIWI_WRAP(sum_array_const_ptr);
    Value wrapped_cst_ref = KIWI_WRAP(sum_array_const_ref);
    Value wrapped_ptr     = KIWI_WRAP(sum_array_ptr);
    Value wrapped_ref     = KIWI_WRAP(sum_array_ref);
    Value wrapped_cpy     = KIWI_WRAP(sum_array_cpy);

    Array<float> v = {1, 2, 3, 4};

    auto value = make_value<Array<float>>(v);
    Value copy            = value;

    REQUIRE(copy.is_valid<Array<float>>() == true);
    REQUIRE(copy.pointer<Array<float>>() == value.pointer<Array<float>>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.is_valid<Array<float>*>() == true);
    REQUIRE(copy.as<Array<float>*>() == value.pointer<Array<float>>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.is_valid<Array<float> const*>() == true);
    REQUIRE(copy.as<Array<float> const*>() == value.pointer<Array<float>>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.is_valid<Array<float>&>() == true);
    // Taking the address of a reference is UB
    REQUIRE(&copy.as<Array<float>&>() == value.pointer<Array<float>>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(copy.is_valid<Array<float> const&>() == true);
    REQUIRE(&copy.as<Array<float> const&>() == value.pointer<Array<float>>());
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped_cst_ptr, value).as<float>() == 10.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped_cst_ref, copy).as<float>() == 10.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped_ptr, copy).as<float>() == 10.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped_ref, copy).as<float>() == 10.0);
    REQUIRE(Value::has_error() == false);

    REQUIRE(invoke(nullptr, wrapped_cpy, copy).as<float>() == 10.0);
    REQUIRE(Value::has_error() == false);

    value.destroy();
    value.destroy();
}

TEST_CASE("Value_Array Copy") {
    Value wrapped = KIWI_WRAP(sum_array_const_ptr);

    Array<float> v = {1, 2, 3, 4};

    auto value = make_value<Array<float>>(v);

    Value shallow_copy = value;

    Value deep_copy = _copy<Array<float>>::copy(value);

    REQUIRE(invoke(nullptr, wrapped, value).as<float>() == 10.0);
    REQUIRE(invoke(nullptr, wrapped, shallow_copy).as<float>() == 10.0);
    REQUIRE(invoke(nullptr, wrapped, deep_copy).as<float>() == 10.0);

    shallow_copy.as<Array<float>&>().push_back(10);

    REQUIRE(invoke(nullptr, wrapped, value).as<float>() == 20.0);
    REQUIRE(invoke(nullptr, wrapped, shallow_copy).as<float>() == 20.0);
    REQUIRE(invoke(nullptr, wrapped, deep_copy).as<float>() == 10.0);

    REQUIRE(shallow_copy.as<Array<float>&>().size() == value.as<Array<float>&>().size());
    REQUIRE(shallow_copy.as<Array<float>&>().size() != deep_copy.as<Array<float>&>().size());

    deep_copy.as<Array<float>&>().push_back(12);
    REQUIRE(shallow_copy.as<Array<float>&>().size() == deep_copy.as<Array<float>&>().size());

    std::cout << is_streamable<std::ostream, Array<float> const&>::value << std::endl;
    std::cout << is_streamable<std::ostream, Array<float>>::value << std::endl;
    std::cout << is_streamable<std::ostream, Array<float>&>::value << std::endl;

    auto printer = [](std::ostream& out, Value const& v) { out << v.as<Array<float> const&>(); };

    register_value<Array<float>>(printer);

    deep_copy.print(std::cout) << "\n";

    value.destroy();
    deep_copy.destroy();
}

TEST_CASE("Value_int ref") {

    Value v(int(1));
    Value ref = _ref<int>::ref(v);

    REQUIRE(v.as<int>() == 1);
    REQUIRE(ref.as<int>() == 1);

    v.as<int&>() = 2;
    REQUIRE(v.as<int>() == 2);
    REQUIRE(ref.as<int>() == 2);
}

//
//
//  VALUE WITH OWING TYPE
//
//
// Not sure if this is worth it
// Maybe assume everything is owning unless it is a pointer ?
// but we DO have owning pointer like C objects
//
//
namespace internal {
template <typename T, bool owned>
struct Claim {
    T data;

    Claim() {}

    template <typename... Args>
    Claim(Args... args): data(args...) {}

    operator T() { return data; }

    operator T&() { return data; }

    operator T const &() const { return data; }

    operator T*() { return &data; }

    operator T const *() const { return &data; }
};
}  // namespace internal

template <typename T>
bool is_owned(Value& val) {
    using Owned = internal::Claim<T, true>;
    return val.is_type<Owned>();
}

template <typename T>
bool is_weakref(Value& val) {
    using Weakref = internal::Claim<T*, false>;
    return val.is_type<Weakref>();
}

template <typename T>
bool is(Value& val) {
    // this is 3 integer compare
    return is_owned<T>(val) || is_weakref<T>(val) || val.is_type<T>();
}

// Template metafunction to strip qualifiers from a type
template <typename T>
struct strip_qualifiers {
    using type = typename std::remove_pointer<typename std::remove_const<
        typename std::remove_reference<typename std::remove_pointer<T>::type>::type>::type>::type;
};

// Template metafunction to transfer qualifiers from type T to type V
template <typename T, typename V>
struct transfer_qualifiers {
    using Base = typename strip_qualifiers<T>::type;

    using type =
        typename std::conditional_t<
            std::is_same_v<Base*, T>,
            V*,
        typename std::conditional_t<
            std::is_same_v<Base const&, T>,
            V const&,
        typename std::conditional_t<
            std::is_same_v<Base const*, T>,
            V const*,
        typename std::conditional_t<
            std::is_same_v<Base&, T>,
            V&,
        typename std::conditional_t<
            std::is_same_v<Base*, T>,
            V*,
        typename std::conditional_t<
            std::is_same_v<Base const, T>,
            V const,
            V
        >
        >
        >
        >
        >
        >;
};

template <typename T>
T as(Value& val, bool& success) {
    // What if T* IS the right type maybe another Ptr wrapper ?
    using BaseType = typename strip_qualifiers<T>::type;

    if (is_owned<BaseType>(val)) {
        using BaseOwned = internal::Claim<BaseType, true>;
        using Owned     = typename transfer_qualifiers<T, BaseOwned>::type;

        success         = true;
        if constexpr (std::is_pointer_v<Owned>) {
            return val.as<Owned>()->operator T();
        } else {
            return val.as<Owned>().operator T();
        }
    }

    if (is_weakref<BaseType>(val)) {
        using BaseWeakref = internal::Claim<BaseType*, false>;
        using Weakref     = typename transfer_qualifiers<T, BaseWeakref>::type;

        success = true;
        if constexpr (std::is_pointer_v<Weakref>) {
            return val.as<Weakref>()->operator T();
        } else {
            return val.as<Weakref>().operator T();
        }
    }

    if (is_owned<BaseType*>(val)) {
        using BaseOwned = internal::Claim<BaseType*, true>;
        using Owned     = typename transfer_qualifiers<T, BaseOwned>::type;

        success = true;
        if constexpr (std::is_pointer_v<Owned>) {
            return val.as<Owned>()->operator T();
        } else {
            return val.as<Owned>().operator T();
        }
    }

    if (is_weakref<BaseType*>(val)) {
        using BaseWeakref = internal::Claim<BaseType*, false>;
        using Weakref     = typename transfer_qualifiers<T, BaseWeakref>::type;

        success = true;
        if constexpr (std::is_pointer_v<Weakref>) {
            return val.as<Weakref>()->operator T();
        } else {
            return val.as<Weakref>().operator T();
        }
    }

    success = false;
    return T();
}

template <typename T, typename... Args>
Value make_owned_value(Args... args) {
    using Ptr = typename std::conditional_t<std::is_pointer_v<T>, T, T*>;

    using Owned   = internal::Claim<T, true>;
    using Weakref = internal::Claim<Ptr, false>;

    auto val = make_value<Owned>(args...);

    ValueRef refmaker = [](Value& original) {
        bool success = false;
        Ptr  ptr     = as<Ptr>(original, success);
        if (success) {
            auto ref = make_value<Weakref>(ptr);
            return ref;
        }
        return Value();
    };

    meta::ClassMetadata& metadata = meta::classmeta(meta::type_id<Owned>());
    // the deleter should alredy be good
    metadata.ref                  = refmaker;
    return val;
}


template <typename T, FreeFun fun, typename... Args>
Value owned_from_pointer(T* raw) {
    using Owned   = internal::Claim<T*, true>;
    using Weakref = internal::Claim<T*, false>;

    auto val = make_value<Owned>(raw);

    ValueRef refmaker = [](Value& original) {
        bool success = false;
        T*  ptr     = as<T*>(original, success);
        if (success) {
            auto ref = make_value<Weakref>(ptr);
            return ref;
        }
        return original;
    };

    ValueDeleter deleter = [](void*, Value& original) {
        bool success = false;
        T* ptr = as<T*>(original, success);
        if (success) {
            fun(ptr);
        }
        return;
    };

    meta::ClassMetadata& metadata = meta::classmeta(val.tag);
    metadata.deleter              = deleter;
    metadata.ref                  = refmaker;
    return val;
}

Value weakref(Value& val) {
    meta::ClassMetadata& metadata = meta::classmeta(val.tag);

    if (metadata.ref) {
        return metadata.ref(val);
    }

    return val;
}

TEST_CASE("Value_Owned_Weakref") {

    make_value<Array<int>>(32);

    Value a = make_owned_value<Array<int>>(32);

    Value ref     = weakref(a);
    bool  success = false;

    Array<int>* ptr_a = as<Array<int>*>(a, success);
    REQUIRE(success);
    REQUIRE(ptr_a->size() == 32);

    Array<int>* ptr_ref = as<Array<int>*>(ref, success);
    REQUIRE(success);
    REQUIRE(ptr_ref->size() == 32);

    REQUIRE(ptr_ref == ptr_a);

    // This should do NOTHING
    ref.destroy();

    // free the value
    a.destroy();
}

TEST_CASE("Value_Owned_Weakref_Pointer") {

    int* owned_array = (int*)malloc(sizeof(int) * 32);

    Value a = owned_from_pointer<int, free>(owned_array);

    Value ref     = weakref(a);
    bool  success = false;

    int* ptr_a = as<int*>(a, success);
    REQUIRE(success);

    int* ptr_ref = as<int*>(ref, success);
    REQUIRE(success);

    REQUIRE(ptr_ref == ptr_a);

    // This should do NOTHING
    ref.destroy();

    // free the value
    a.destroy();
}

TEST_CASE("Transfer") {
    {
        using T = const int*;
        using V = double;
        using E = const double*;

        using R = transfer_qualifiers<T, V>::type;
        static_assert(std::is_same<R, E>::value, "Transfer the qualifier");
    }

    {
        using T = const int*;
        using V = double;
        using E = const double*;

        using R = transfer_qualifiers<T, V>::type;
        static_assert(std::is_same<R, E>::value, "Transfer the qualifier");
    }

    {
        using T = int;
        using V = double;
        using E = double;

        using R = transfer_qualifiers<T, V>::type;
        static_assert(std::is_same<R, E>::value, "Transfer the qualifier");
    }

    {
        using T = int const;
        using V = double;
        using E = double const;

        using R = transfer_qualifiers<T, V>::type;
        static_assert(std::is_same<R, E>::value, "Transfer the qualifier");
    }

    {
        using T = int&;
        using V = double;
        using E = double&;

        using R = transfer_qualifiers<T, V>::type;
        static_assert(std::is_same<R, E>::value, "Transfer the qualifier");
    }

    {
        using T = int const&;
        using V = double;
        using E = double const&;

        using R = transfer_qualifiers<T, V>::type;
        static_assert(std::is_same<R, E>::value, "Transfer the qualifier");
    }
}



TEST_CASE("Value_Point_attribute") {
    meta::register_members<Point2D>();

    {
        auto value = make_value<Point2D>(3.0f, 4.0f);

        meta::ClassMetadata const& meta = meta::classmeta(value.tag);
        REQUIRE(meta.members.size() == 2);

        Value x = getattr(value, "x");
        REQUIRE(x.tag == meta::type_id<float>());
        REQUIRE(x.as<float>() == 3.f);

        Value y = getattr(value, "y");
        REQUIRE(y.tag == meta::type_id<float>());
        REQUIRE(y.as<float>() == 4.f);

        setattr(value, "y", make_value<float>(5.0f));

        // Point2D instance was changed
        y = getattr(value, "y");
        REQUIRE(y.tag == meta::type_id<float>());
        REQUIRE(y.as<float>() == 5.f);

        // Point2D instance was changed
        REQUIRE(value.as<Point2D>().y == 5.f);
    }

    {
        auto value = make_value<Point2D>(3.0f, 4.0f);

        Value y = getattrref(value, "y");
        REQUIRE(y.tag == meta::type_id<float*>());
        REQUIRE(y.as<float>() == 4.f);

        y.ref<float>() = 5.0f;

        // Point2D instance was changed
        y = getattr(value, "y");
        REQUIRE(y.tag == meta::type_id<float>());
        REQUIRE(y.as<float>() == 5.f);

        // Point2D instance was changed
        REQUIRE(value.as<Point2D>().y == 5.f);
    }
}

TEST_CASE("Value attribute") {
    meta::register_members<Rectangle>();

    auto value = make_value<Rectangle>(Point2D(3.0f, 4.0f), Point2D(3.0f, 4.0f));
 
     meta::ClassMetadata const& meta = meta::classmeta(value.tag);

    REQUIRE(meta.members.size() == 2);

    Value p = getattr(value, "p");
    REQUIRE(p.tag == meta::type_id<Point2D>());
    REQUIRE(p.as<Point2D>() == Point2D(3.0f, 4.0f));

    Value s = getattr(value, "s");
    REQUIRE(s.tag == meta::type_id<Point2D>());
    REQUIRE(s.as<Point2D>() == Point2D(3.0f, 4.0f));

    setattr(value, "s", make_value<Point2D>(1.0f, 2.0f));
    s = getattr(value, "s");
    REQUIRE(s.tag == meta::type_id<Point2D>());
    REQUIRE(s.as<Point2D>() == Point2D(1.0f, 2.0f));

}

struct Rect2 {
    Rectangle a;
    Rectangle b;

    Rect2() {}

    Rect2(Rectangle p, Rectangle s): a(p), b(s) {}

};


template <>
struct lython::meta::ReflectionTrait<Rect2> {
    static int register_members() {
        lython::meta::new_member<Rect2, Rectangle>("a");
        lython::meta::new_member<Rect2, Rectangle>("b");
        // lython::meta::new_method("add", &Pnt::add);
        // lython::meta::new_method("sum", &Pnt::sum);

        return 1;
    }
};


TEST_CASE("Value Rect2 attribute") {
    meta::register_members<Rect2>();

    auto rect1 = Rectangle(Point2D(1.0f, 2.0f), Point2D(3.0f, 4.0f));
    auto rect2 = Rectangle(Point2D(5.0f, 6.0f), Point2D(7.0f, 8.0f));

    auto value = make_value<Rect2>(rect1, rect2);
 
    meta::ClassMetadata const& meta = meta::classmeta(value.tag);

    REQUIRE(meta.members.size() == 2);

    Value p = getattr(value, "a");
    REQUIRE(p.tag == meta::type_id<Rectangle>());
    REQUIRE(p.as<Rectangle>() == rect1);

    Value s = getattr(value, "b");
    REQUIRE(s.tag == meta::type_id<Rectangle>());
    REQUIRE(s.as<Rectangle>() == rect2);

    auto rect3 = Rectangle(Point2D(9.0f, 10.0f), Point2D(11.0f, 12.0f));
    setattr(value, "b", make_value<Rectangle>(rect3));
    s = getattr(value, "b");
    REQUIRE(s.tag == meta::type_id<Rectangle>());
    REQUIRE(s.as<Rectangle>() == rect3);
}


struct NewVec {
    NewVec(Array<float> const& data):
        data(data)
    {}

    Array<float> data;
};


template <>
struct lython::meta::ReflectionTrait<NewVec> {
    static int register_members() {
        lython::meta::new_member<NewVec, Array<float>>("data");
        return 1;
    }
};


TEST_CASE("Value Non trivial attribute") {
    meta::register_members<NewVec>();

    Array<float> data = {1.f, 2.f};
    auto value = make_value<NewVec>(data);
 
    meta::ClassMetadata& meta = meta::classmeta(value.tag);

    {
        meta::ClassMetadata& meta_v = meta::classmeta(meta::type_id<Array<float>>());
        meta_v.assign = [](Value& src, Value const& dest) {
            src.as<Array<float>&>() = dest.as<Array<float> const&>();
        };
    }

    REQUIRE(meta.members.size() == 1);

    Value p = getattr(value, "data");
    REQUIRE(p.tag == meta::type_id<Array<float>>());
    REQUIRE(p.as<Array<float>>() == data);

    Array<float> data2 = {3.f, 4.f};
    setattr(value, "data", make_value<Array<float>>(data2));
    Value s = getattr(value, "data");
    REQUIRE(s.tag == meta::type_id<Array<float>>());
    REQUIRE(s.as<Array<float>>() == data2);
}

// heap is taken care of by the GC
// stack we use pthread to fetch the range
// 
TEST_CASE("Value Non trivial attribute2") {
    meta::register_members<NewVec>();

    {
        Array<float> data = {1.f, 2.f};
        auto value = make_value<NewVec>(data);
    }
}