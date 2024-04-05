#include "ast/values/value.h"
#include <iostream>
#include <algorithm>

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
}

// This struct is small enough and will be stored on the stack
struct Point2D {
    Point2D() {}

    Point2D(float xx, float yy): x(xx), y(yy) {}

    float x = 0;
    float y = 0;

    float distance() const { return sqrt(x * x + y * y); }

    float distance2() { return sqrt(x * x + y * y); }
};

float freefun_distance(Point2D const* p) { return sqrt(p->x * p->x + p->y * p->y); }

TEST_CASE("Value_SVO_Function Wrapping") {
    Value distance([](void*, Array<Value>& args) -> Value {
        Value a = args[0];
        return Value(a.pointer<Point2D>()->distance());
    });
    Value wrapped      = KIWI_WRAP(freefun_distance);
    Value method       = KIWI_WRAP(Point2D::distance2);
    Value const_method = KIWI_WRAP(Point2D::distance);

    auto [value, deleter] = make_value<Point2D>(3.0f, 4.0f);
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

    REQUIRE(deleter == noop_destructor);
}

// This struct is too big and it will be allocated on the heap
struct Rectangle {
    Rectangle() {}

    Rectangle(Point2D p, Point2D s): p(p), s(s) {}

    Point2D p;
    Point2D s;

    float perimeter() const { return (s.x + s.y) * 2.0f; }

    float perimeter2() { return (s.x + s.y) * 2.0f; }
};

float freefun_perimeter_cst    (Rectangle const* p) { return (p->s.x + p->s.y) * 2.0f;  }
float freefun_perimeter_cst_ref(Rectangle const& p) { return (p.s.x + p.s.y) * 2.0f;    }
float freefun_perimeter        (Rectangle* p)       { return (p->s.x + p->s.y) * 2.0f;  }
float freefun_perimeter_ref    (Rectangle& p)       { return (p.s.x + p.s.y) * 2.0f;    }
float freefun_perimeter_cpy    (Rectangle p)        { return (p.s.x + p.s.y) * 2.0f;    }


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

    auto [value, deleter] = make_value<Rectangle>(Point2D(3.0f, 4.0f), Point2D(3.0f, 4.0f));
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

    deleter(value);
}

TEST_CASE("Value_Reference") {

    int i = 4;

    auto [value, deleter] = make_value<int*>(&i);

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

    REQUIRE(deleter == noop_destructor);
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

    auto [value, deleter] = from_pointer<cstruct, free_cstruct>(ptr);

    REQUIRE(value.is_valid<cstruct*>() == true);
    REQUIRE(value.as<cstruct*>() == ptr);
    REQUIRE(Value::has_error() == false);
    
    REQUIRE(value.as<cstruct*>()->a == 2.0f);
    REQUIRE(Value::has_error() == false);

    deleter(value);
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
    std::cout << sizeof(Value::Holder) << "\n";
    std::cout << sizeof(ScriptObjectFixed<1>) << "\n";
    std::cout << sizeof(ScriptObjectFixed<2>) << "\n";
    std::cout << sizeof(ScriptObjectFixed<3>) << "\n";
    std::cout << sizeof(ScriptObjectFixed<4>) << "\n";
    std::cout << sizeof(ScriptObjectTest) << "\n";
}

TEST_CASE("Value_ErrorHandling") 
{
    Value a(1);

    REQUIRE(a.is_valid<float>() == false);
    REQUIRE(a.is_valid<int>() == true);

    a.as<float>();
    REQUIRE(Value::has_error() == true);
    REQUIRE(Value::global_err.requested_type_id == meta::type_id<float>());
    REQUIRE(Value::global_err.value_type_id == meta::type_id<int>());
    Value::reset_error();
}




#define SUM(x) std::accumulate((x).begin(), (x).end(), 0.f)

float sum_array_const_ptr(Array<float> const* p) { return SUM(*p);  }
float sum_array_const_ref(Array<float> const& p) { return SUM(p);    }
float sum_array_ptr      (Array<float>* p)       { return SUM(*p);  }
float sum_array_ref      (Array<float>& p)       { return SUM(p);    }
float sum_array_cpy      (Array<float> p)        { return SUM(p);    }


TEST_CASE("Value_Array Wrapping") {
    Value wrapped_cst_ptr = KIWI_WRAP(sum_array_const_ptr);
    Value wrapped_cst_ref = KIWI_WRAP(sum_array_const_ref);
    Value wrapped_ptr     = KIWI_WRAP(sum_array_ptr);
    Value wrapped_ref     = KIWI_WRAP(sum_array_ref);
    Value wrapped_cpy     = KIWI_WRAP(sum_array_cpy);

    Array<float> v = {
        1,
        2,
        3,
        4
    };

    auto [value, deleter] = make_value<Array<float>>(v);
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

    deleter(value);
}
