#include "doctest.h"

#include <climits>
#include <cstdint>

#include "mystery.hpp"

static std::uint32_t next(std::uint32_t& state) {
    state = state * 1664525 + 1013904223;
    return state;
}

TEST_CASE("quebec divides by ten") {
    for (unsigned x = 0; x < 2000; ++x) {
        CAPTURE(x);
        REQUIRE(quebec(x) == x / 10);
    }
    std::uint32_t state = 31;
    for (int i = 0; i < 5000; ++i) {
        unsigned x = next(state);
        CAPTURE(x);
        REQUIRE(quebec(x) == x / 10);
    }
    CHECK(quebec(UINT_MAX) == 429496729);
    CHECK(quebec(UINT_MAX - 5) == 429496729);
    CHECK(quebec(UINT_MAX - 6) == 429496728);
    CHECK(quebec(2147483648u) == 214748364);
}

TEST_CASE("romeo multiplies by fifteen") {
    std::uint32_t state = 32;
    for (int i = 0; i < 2000; ++i) {
        long x = static_cast<long>(next(state) >> 8) - (1 << 23);
        CAPTURE(x);
        REQUIRE(romeo(x) == x * 15);
    }
    CHECK(romeo(0) == 0);
    CHECK(romeo(-1) == -15);
    CHECK(romeo(100000000000) == 1500000000000);
}

TEST_CASE("sierra divides by four, rounding toward zero") {
    for (int x = -200; x <= 200; ++x) {
        CAPTURE(x);
        REQUIRE(sierra(x) == x / 4);
    }
    CHECK(sierra(-1) == 0);
    CHECK(sierra(-3) == 0);
    CHECK(sierra(-4) == -1);
    CHECK(sierra(-5) == -1);
    CHECK(sierra(INT_MAX) == 536870911);
    CHECK(sierra(INT_MIN) == -536870912);
    CHECK(sierra(INT_MIN + 1) == -536870911);
}

TEST_CASE("tango is true for the ten digits and nothing else") {
    for (int v = -128; v <= 127; ++v) {
        char c = static_cast<char>(v);
        CAPTURE(v);
        REQUIRE(tango(c) == (v >= '0' && v <= '9'));
    }
}

TEST_CASE("uniform for every month, and for what isn't one") {
    const int before[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
    for (int month = 1; month <= 12; ++month) {
        CAPTURE(month);
        REQUIRE(uniform(month) == before[month - 1]);
    }
    for (int month : {0, -1, -11, 13, 14, 100, INT_MAX, INT_MIN}) {
        CAPTURE(month);
        REQUIRE(uniform(month) == -1);
    }
}
