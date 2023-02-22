#include <catch2/catch_all.hpp>

#include "utilities/strings.h"

using namespace lython;

TEST_CASE("strings") {
    SECTION("join") {
        REQUIRE(join(".", Array<String>{"a", "b", "c"}) == "a.b.c");
        REQUIRE(join(".", Array<String>{"a", "b"}) == "a.b");
        REQUIRE(join(".", Array<String>{"a"}) == "a");
        REQUIRE(join(".", Array<String>{}) == "");
    }

    SECTION("strip") {
        REQUIRE(strip(" a.b ") == "a.b");
        REQUIRE(strip("\na.b\n") == "a.b");
        REQUIRE(strip("\ta.b \n\t") == "a.b");
        REQUIRE(strip("\ta. .b \n\t") == "a. .b");
        REQUIRE(strip("\n\ta. .b \n\n") == "a. .b");
    }

    SECTION("split") {
        REQUIRE(split('.', "a.b.c") == Array<String>{"a", "b", "c"});
        REQUIRE(split('.', "a") == Array<String>{"a"});
        REQUIRE(split('.', ".b.c") == Array<String>{"", "b", "c"});
        REQUIRE(split('.', "a..c") == Array<String>{"a", "", "c"});
        REQUIRE(split('.', "a.b.") == Array<String>{"a", "b", ""});
        REQUIRE(split('.', "..") == Array<String>{"", "", ""});
        REQUIRE(split('.', "") == Array<String>{""});
    }
}
