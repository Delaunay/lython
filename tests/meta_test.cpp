
#include <iostream>
#include <sstream>

#include "ast/values/native.h"
#include "utilities/debug.h"
#include "utilities/metadata.h"
#include "utilities/metadata_method.h"

#include <catch2/catch_all.hpp>

struct Point {
    float x = 3;
    int   y = 3;

    float sum(float z) { return x + y + z; }
};

template <>
struct lython::meta::ReflectionTrait<Point> {
    static int register_members() {
        lython::meta::new_member<Point, float>("x");
        lython::meta::new_member<Point, int>("y");
        lython::meta::new_method("sum", &Point::sum);

        return 0;
    }
};

template <typename T>
void set_printer(std::function<void(std::ostream&, std::int8_t const*)> const& printer) {
    auto& klass   = lython::meta::classmeta<T>();
    klass.printer = printer;
}

TEST_CASE("NATIVE Object") {

    set_printer<int>([](std::ostream& out, std::int8_t const* data) {
        out << *reinterpret_cast<int const*>(data);
    });

    set_printer<float>([](std::ostream& out, std::int8_t const* data) {
        out << *reinterpret_cast<float const*>(data);
    });

    auto   wrapped = lython::NativeValue<Point>();
    Point* p       = wrapped.as<Point>();

    std::stringstream ss;
    lython::meta::print(ss, *p);
    std::cout << ss.str() << "\n";

    auto wp = lython::NativePointer<Point>(wrapped);
    REQUIRE(p->sum(0) == 6);
    REQUIRE(wp.as<Point>()->sum(0) == 6);

    float* x = wrapped.member<float>("x");
    int*   y = wrapped.member<int>("y");

    (*x) = 1.0;
    (*y) = 2;

    p = wrapped.as<Point>();
    REQUIRE(p->sum(0) == 3);
    REQUIRE(wp.as<Point>()->sum(0) == 3);

    // This is a bit too much info to provide
    // float r = wrapped.invoke<float, float>(p, "sum", {&arg});

    {
        // this is nice
        auto                  arg    = lython::NativeValue<float>(10);
        lython::NativeObject* r      = wrapped.invoke(p, "sum", {&arg});
        float                 result = *((lython::NativeValue<float>*)(r))->as<float>();
        std::cout << "Result: " << result << std::endl;
        REQUIRE(wp.as<Point>()->sum(10) == result);
    }

    try {
        // How does this fail ?
        auto                  arg    = lython::NativeValue<int>(10);
        lython::NativeObject* r      = wrapped.invoke(p, "sum", {&arg});
    } catch (lython::WrongTypeException& e) {
         std::cout << "Error " << e.what() << std::endl;
    }

    std::stringstream ss2;
    lython::meta::print(ss2, *p);
    std::cout << ss2.str() << "\n";
}

TEST_CASE("Debug Print") {

    print(std::cout, std::tuple<std::string, int>{"k1", 1});
    std::cout << "\n";

    print(std::cout,
          std::vector<std::string>{
              "k1",
              "k2",
              "k3",
              "k4",
              "k5",
          });
    std::cout << "\n";

    print(std::cout,
          std::unordered_map<std::string, int>{
              {"k1", 1},
              {"k2", 2},
              {"k3", 3},
              {"k4", 4},
              {"k5", 5},
          });
    std::cout << "\n";

    std::unordered_map<std::string, int> hshtable{
        {"k1", 1},
        {"k2", 2},
        {"k3", 3},
        {"k4", 4},
        {"k5", 5},
    };

    LYT_SHOW(std::cout, hshtable);
}