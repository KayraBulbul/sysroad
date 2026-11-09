#include "doctest.h"

#include "bench.hpp"

TEST_CASE("median") {
    CHECK(median({3, 1, 2}) == 2);
    CHECK(median({4, 1, 3, 2}) == 2.5);
    CHECK(median({7}) == 7);
}

TEST_CASE("ns_per_call calls f reps × samples times") {
    int calls = 0;
    double ns = ns_per_call([&] { ++calls; }, 10, 5);
    CHECK(calls == 50);
    CHECK(ns >= 0);
}
