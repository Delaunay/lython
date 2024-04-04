#include "ast/values/value.h"
#include <iostream>

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

    REQUIRE(copy.pointer<Point2D>() != value.pointer<Point2D>());

    // Retrieve a copy of the value
    REQUIRE(copy.as<Point2D>().x == 3.0f);
    REQUIRE(copy.as<Point2D>().y == 4.0f);

    // Retrieve a pointer of the value
    REQUIRE(copy.as<Point2D*>()->y == 4.0f);
    REQUIRE(copy.as<Point2D const*>()->y == 4.0f);

    REQUIRE(invoke(nullptr, distance, value).as<float>() == 5.0f);
    REQUIRE(invoke(nullptr, wrapped, value).as<float>() == 5.0f);
    REQUIRE(invoke(nullptr, method, value).as<float>() == 5.0f);
    REQUIRE(invoke(nullptr, const_method, value).as<float>() == 5.0f);

    REQUIRE(invoke(nullptr, distance, copy).as<float>() == 5.0f);
    REQUIRE(invoke(nullptr, wrapped, copy).as<float>() == 5.0f);
    REQUIRE(invoke(nullptr, method, copy).as<float>() == 5.0f);
    REQUIRE(invoke(nullptr, const_method, copy).as<float>() == 5.0f);

    deleter(value);
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

float freefun_perimeter(Rectangle const* p) { return (p->s.x + p->s.y) * 2.0f; }

TEST_CASE("Value_NOSVO_Function Wrapping") {
    Value distance([](void*, Array<Value>& args) -> Value {
        Value a = args[0];
        return Value(a.as<Rectangle>().perimeter());
    });

    Value wrapped      = KIWI_WRAP(freefun_perimeter);
    Value method       = KIWI_WRAP(Rectangle::perimeter2);
    Value const_method = KIWI_WRAP(Rectangle::perimeter);

    auto [value, deleter] = make_value<Rectangle>(Point2D(3.0f, 4.0f), Point2D(3.0f, 4.0f));
    Value copy            = value;

    REQUIRE(copy.pointer<Rectangle>() == value.pointer<Rectangle>());

    REQUIRE(invoke(nullptr, distance, value).as<float>() == 14.0);
    REQUIRE(invoke(nullptr, wrapped, value).as<float>() == 14.0);
    REQUIRE(invoke(nullptr, method, value).as<float>() == 14.0);
    REQUIRE(invoke(nullptr, const_method, value).as<float>() == 14.0);

    REQUIRE(invoke(nullptr, distance, copy).as<float>() == 14.0);
    REQUIRE(invoke(nullptr, wrapped, copy).as<float>() == 14.0);
    REQUIRE(invoke(nullptr, method, copy).as<float>() == 14.0);
    REQUIRE(invoke(nullptr, const_method, copy).as<float>() == 14.0);

    deleter(value);
}

TEST_CASE("Value_Reference") {

    int i = 4;

    auto [value, deleter] = make_value<int*>(&i);

    std::cout << (&i) << std::endl;
    std::cout << (value.value.obj) << std::endl;
    std::cout << (value.as<int*>()) << std::endl;
    std::cout << *(value.as<int**>()) << std::endl;

    REQUIRE((**value.as<int**>()) == 4);
    i = 5;
    REQUIRE((**value.as<int**>()) == 5);
}