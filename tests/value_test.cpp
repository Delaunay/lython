#include "ast/values/value.h"

#include <catch2/catch_all.hpp>

using namespace lython;

// This struct is small enough and will be stored on the stack
struct Point {
    Point(float xx, float yy): x(xx), y(yy) {}

    float x = 0;
    float y = 0;

    float distance() const { return sqrt(x * x + y * y); }

    float distance2() { return sqrt(x * x + y * y); }
};

float freefun_distance(Point const* p) { return sqrt(p->x * p->x + p->y * p->y); }

TEST_CASE("Value_SVO_Function Wrapping") {
    Value distance([](void*, Array<Value>& args) -> Value {
        Value a = args[0];
        return Value(a.as<Point*>()->distance());
    });
    Value wrapped      = KIWI_WRAP(freefun_distance);
    Value method       = KIWI_WRAP(Point::distance2);
    Value const_method = KIWI_WRAP(Point::distance);

    auto [value, deleter] = make_value<Point>(3.0, 4.0);
    Value copy            = value;

    REQUIRE(copy.as<Point*>() != value.as<Point*>());

    REQUIRE(copy.as<Point*>()->x == 3.0);
    REQUIRE(copy.as<Point*>()->y == 4.0);

    REQUIRE(invoke(nullptr, distance, value) == 5.0);
    REQUIRE(invoke(nullptr, wrapped, value) == 5.0);
    REQUIRE(invoke(nullptr, method, value) == 5.0);
    REQUIRE(invoke(nullptr, const_method, value)  == 5.0);

    REQUIRE(invoke(nullptr, distance, copy) == 5.0);
    REQUIRE(invoke(nullptr, wrapped, copy) == 5.0);
    REQUIRE(invoke(nullptr, method, copy) == 5.0);
    REQUIRE(invoke(nullptr, const_method, copy) == 5.0);
}


// This struct is too big and it will be allocated on the heap
struct Rectangle {
    Rectangle(Point p, Point s): p(p), s(s) {}

    Point p;
    Point s;

    float perimeter() const { return (s.x + s.y) * 2.0; }

    float perimeter2() { return (s.x + s.y) * 2.0; }
};

float freefun_perimeter(Rectangle const* p) { return (p->s.x + p->s.y) * 2.0; }


TEST_CASE("Value_NOSVO_Function Wrapping") {
    Value distance([](void*, Array<Value>& args) -> Value {
        Value a = args[0];
        return Value(a.as<Rectangle*>()->perimeter());
    });

    Value wrapped      = KIWI_WRAP(freefun_perimeter);
    Value method       = KIWI_WRAP(Rectangle::perimeter2);
    Value const_method = KIWI_WRAP(Rectangle::perimeter);

    auto [value, deleter] = make_value<Rectangle>(Point(3.0, 4.0), Point(3.0, 4.0));
    Value copy            = value;

    REQUIRE(copy.as<Rectangle*>() == value.as<Rectangle*>());

    REQUIRE(invoke(nullptr, distance, value) == 5.0);
    REQUIRE(invoke(nullptr, wrapped, value) == 5.0);
    REQUIRE(invoke(nullptr, method, value) == 5.0);
    REQUIRE(invoke(nullptr, const_method, value)  == 5.0);

    REQUIRE(invoke(nullptr, distance, copy) == 5.0);
    REQUIRE(invoke(nullptr, wrapped, copy) == 5.0);
    REQUIRE(invoke(nullptr, method, copy) == 5.0);
    REQUIRE(invoke(nullptr, const_method, copy) == 5.0);

    deleter(value);
}
