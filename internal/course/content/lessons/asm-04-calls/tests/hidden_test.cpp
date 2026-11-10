#include "doctest.h"

#include <cstdint>

#include "mystery.hpp"

static long pick(std::uint64_t& state) {
    state = state * 6364136223846793005u + 1442695040888963407u;
    return static_cast<long>(state >> 50) - (1 << 13);
}

TEST_CASE("foxtrot uses a, g and h, and nothing else") {
    std::uint64_t state = 11;
    for (int i = 0; i < 1000; ++i) {
        long v[8];
        for (long& x : v) {
            x = pick(state);
        }
        CAPTURE(v[0]);
        CAPTURE(v[6]);
        CAPTURE(v[7]);
        REQUIRE(foxtrot(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]) == (v[0] + v[6]) * v[7]);
    }
    CHECK(foxtrot(0, 9, 9, 9, 9, 9, 0, 9) == 0);
    CHECK(foxtrot(1, 0, 0, 0, 0, 0, 0, 1) == 1);
    CHECK(foxtrot(0, 0, 0, 0, 0, 0, 1, 1) == 1);
}

TEST_CASE("golf and hotel") {
    std::uint64_t state = 12;
    for (int i = 0; i < 1000; ++i) {
        long a = pick(state);
        long b = pick(state);
        CAPTURE(a);
        CAPTURE(b);
        REQUIRE(golf(a) == a * a + 1);
        REQUIRE(hotel(a, b) == a * a + b * b);
    }
    CHECK(golf(0) == 1);
    CHECK(hotel(0, 0) == 0);
    CHECK(hotel(-3, 0) == 9);
    CHECK(hotel(0, -3) == 9);
}

TEST_CASE("india") {
    CHECK(india(-5) == 0);
    CHECK(india(1) == 1);
    CHECK(india(2) == 3);
    CHECK(india(4) == 148);
    CHECK(india(5) == 21909);
    CHECK(india(6) == 480004287);
    CHECK(india(7) == 230404115538378376);
}

TEST_CASE("juliet") {
    std::uint64_t state = 13;
    for (int i = 0; i < 1000; ++i) {
        long seed = pick(state);
        long values[4];
        fill(values, seed);
        CAPTURE(seed);
        REQUIRE(juliet(seed) == values[0] + values[3]);
    }
    CHECK(juliet(0) == 0);
    CHECK(juliet(-1) == -5);
}
