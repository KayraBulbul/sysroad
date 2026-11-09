#include "doctest.h"

#include <climits>
#include <cstdint>
#include <vector>

#include "ub.hpp"

TEST_CASE("midpoint everywhere") {
    CHECK(midpoint(INT_MIN, INT_MIN) == INT_MIN);
    CHECK(midpoint(INT_MIN, INT_MAX) == 0);
    CHECK(midpoint(-3, 0) == -1);  // -1.5 rounds towards zero
    CHECK(midpoint(3, 0) == 1);
    CHECK(midpoint(-7, -8) == -7);
    CHECK(midpoint(10, 20) == 15);
}

TEST_CASE("rotl by every amount") {
    std::uint32_t x = 0x80000001;
    CHECK(rotl(x, 1) == 0x00000003);
    CHECK(rotl(x, 31) == 0xc0000000);
    CHECK(rotl(x, 32) == x);
    CHECK(rotl(x, 33) == 0x00000003);
    CHECK(rotl(x, 64) == x);
    for (unsigned n = 0; n < 70; ++n) {
        CHECK(rotl(rotl(0xdeadbeef, n), 32 - n % 32) == 0xdeadbeef);
    }
}

TEST_CASE("read_u32 at every offset") {
    alignas(8) unsigned char b[12] = {};
    for (int off = 0; off < 8; ++off) {
        for (int i = 0; i < 12; ++i) {
            b[i] = 0;
        }
        b[off] = 0x44;
        b[off + 1] = 0x33;
        b[off + 2] = 0x22;
        b[off + 3] = 0x11;
        CHECK(read_u32(b + off) == 0x11223344);
    }
}

TEST_CASE("has_adjacent_equal at the edges") {
    CHECK_FALSE(has_adjacent_equal({}));
    CHECK_FALSE(has_adjacent_equal({5}));
    CHECK(has_adjacent_equal({5, 5}));
    CHECK(has_adjacent_equal({1, 2, 3, 3}));
    CHECK_FALSE(has_adjacent_equal({3, 1, 3}));
}

TEST_CASE("magnitude everywhere") {
    CHECK(magnitude(0) == 0);
    CHECK(magnitude(-1) == 1);
    CHECK(magnitude(5) == 5);
    CHECK(magnitude(INT_MAX) == 2147483647u);
    CHECK(magnitude(INT_MIN + 1) == 2147483647u);
}
