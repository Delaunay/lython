#include <catch2/catch_all.hpp>
#include <iostream>

#include "utilities/pool.h"

using namespace lython;

TEST_CASE("pool") {
    SECTION("size") {
        ThreadPool pool(2);
        REQUIRE(pool.size() == 2);
    }

    SECTION("insert_worker") {
        ThreadPool pool(2);
        pool.insert_worker();
        REQUIRE(pool.size() == 2 + 1);
    }

    SECTION("schedule") {
        ThreadPool pool(2);

        auto future = pool.queue_task([](int a, int b) { return a + b; }, 1, 2);

        future.wait();

        REQUIRE(future.get() == 3);
    }

    SECTION("report") {
        ThreadPool pool(2);
        for (int i = 0; i < 8; i++) {
            pool.queue_task([]() {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                return true;
            });
        }

        std::this_thread::sleep_for(std::chrono::microseconds(5));
        pool.print(std::cout);
        pool.shutdown(true);
    }

    SECTION("shutdown") {
        ThreadPool pool(2);
        pool.shutdown(true);
        REQUIRE(pool.size() == 0);
    }

    SECTION("restart pool") {
        ThreadPool pool(2);
        pool.shutdown(true);
        REQUIRE(pool.size() == 0);
        pool.insert_worker();
        REQUIRE(pool.size() == 1);
        pool.shutdown(true);
        REQUIRE(pool.size() == 0);
    }
}
