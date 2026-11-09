#include "doctest.h"

#include <chrono>
#include <thread>

#include "bench.hpp"

TEST_CASE("median ignores outliers") {
    CHECK(median({5, 5, 5, 1000, 5}) == 5);
    CHECK(median({1, 1000}) == 500.5);
    CHECK(median({-3, 10, 0}) == 0);
}

TEST_CASE("median doesn't depend on the order") {
    CHECK(median({9, 2, 7, 4, 5, 1, 8}) == 5);
}

TEST_CASE("ns_per_call measures per call, not per batch") {
    double ns =
        ns_per_call([] { std::this_thread::sleep_for(std::chrono::milliseconds(2)); }, 3, 3);
    CHECK(ns > 1.9e6);  // at least the 2 ms sleep
    CHECK(ns < 60e6);   // and not the whole batch of 3
}

TEST_CASE("a slow sample doesn't drag the result") {
    int call = 0;
    double ns = ns_per_call(
        [&] {
            if (++call == 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        },
        1, 5);
    CHECK(ns < 20e6);  // the median of 5, so the one 50 ms call is ignored
}
