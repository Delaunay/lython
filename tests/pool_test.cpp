#include <catch2/catch.hpp>
#include <iostream>

#include "utilities/pool.h"

using namespace lython;


TEST_CASE("pool"){
    ThreadPool pool(2);

    SECTION("size"){
        REQUIRE(pool.size() == 2);
    }

    SECTION("insert_worker"){
        pool.insert_worker();
        REQUIRE(pool.size() == 2 + 1);
    }

    SECTION("schedule"){
        auto future = pool.queue_task([](int a, int b){
            return a + b;
        }, 1, 2);

        future.wait();

        REQUIRE(future.get() == 3);
    }

    SECTION("report"){
        pool.print(std::cout);
    }

    SECTION("shutdown"){
        pool.shutdown(true);
    }

    SECTION("restart pool"){
        pool.insert_worker();
        REQUIRE(pool.size() == 1);
    }

    SECTION("shutdown2"){
        pool.shutdown(true);
    }
}
