#include "doctest.h"

#include <cstdint>

#include "mystery.hpp"

static long pick(std::uint64_t& state) {
    state = state * 6364136223846793005u + 1442695040888963407u;
    // Small enough that nothing overflows, with negatives.
    return static_cast<long>(state >> 44) - (1 << 19);
}

TEST_CASE("first, second, third and fourth on many inputs") {
    std::uint64_t state = 1;
    for (int i = 0; i < 2000; ++i) {
        long a = pick(state);
        long b = pick(state);
        long c = pick(state);
        CAPTURE(a);
        CAPTURE(b);
        CAPTURE(c);
        REQUIRE(first(a, b) == a + b);
        REQUIRE(second(a, b) == a - 2 * b);
        REQUIRE(third(a) == 5 * a + 7);
        REQUIRE(fourth(a, b, c) == (a - b) * c);
    }
}

TEST_CASE("fifth on many inputs") {
    std::uint64_t state = 2;
    for (int i = 0; i < 2000; ++i) {
        int a = static_cast<int>(pick(state));
        int b = static_cast<int>(pick(state));
        CAPTURE(a);
        CAPTURE(b);
        REQUIRE(fifth(a, b) == 3 * a - b + 100);
    }
}

TEST_CASE("sixth on many inputs, negative ones too") {
    std::uint64_t state = 3;
    for (int i = 0; i < 2000; ++i) {
        long a = pick(state);
        long b = pick(state);
        CAPTURE(a);
        CAPTURE(b);
        REQUIRE(sixth(a, b) == a * 16 + (b >> 2));
    }
    CHECK(sixth(0, -1) == -1);
    CHECK(sixth(0, -5) == -2);
    CHECK(sixth(-1, 0) == -16);
}

TEST_CASE("zeros") {
    CHECK(first(0, 0) == 0);
    CHECK(second(0, 0) == 0);
    CHECK(third(0) == 7);
    CHECK(fourth(0, 0, 0) == 0);
    CHECK(fifth(0, 0) == 100);
    CHECK(sixth(0, 0) == 0);
}
