#include <catch2/catch.hpp>

#include "utilities/strings.h"

using namespace lython;

TEST_CASE("strings"){
    SECTION("join"){
        REQUIRE(join(".", {"a", "b"}) == "a.b");
        REQUIRE(join(".", {"a"}) == "a");
        REQUIRE(join(".", {}) == "");
    }

    SECTION("strip"){
        REQUIRE(strip(" a.b ") == "a.b");
        REQUIRE(strip("\na.b\n") == "a.b");
        REQUIRE(strip("\ta.b \n\t") == "a.b");
        REQUIRE(strip("\ta. .b \n\t") == "a. .b");
    }
}
