#include "doctest.h"

#include <cstdint>

#include "twos.hpp"

TEST_CASE("sign_extend at every width") {
    CHECK(sign_extend(1, 1) == -1);
    CHECK(sign_extend(0, 1) == 0);
    CHECK(sign_extend(0b10, 2) == -2);
    CHECK(sign_extend(0b01, 2) == 1);
    CHECK(sign_extend(0xffff, 16) == -1);
    CHECK(sign_extend(0x8000, 16) == -32768);
    CHECK(sign_extend(0x7fff, 16) == 32767);
    CHECK(sign_extend(0xffffffff, 32) == -1);
    CHECK(sign_extend(0x80000000, 32) == INT32_MIN);
    CHECK(sign_extend(0x7fffffff, 32) == INT32_MAX);
    CHECK(sign_extend(0x40000000, 31) == -1073741824);
    for (int bits = 1; bits <= 32; ++bits) {
        CAPTURE(bits);
        // All ones is -1 at any width, and the sign bit alone is the smallest value.
        std::uint32_t ones = bits == 32 ? 0xffffffffu : (1u << bits) - 1;
        CHECK(sign_extend(ones, bits) == -1);
        CHECK(sign_extend(1u << (bits - 1), bits) == -(std::int64_t{1} << (bits - 1)));
    }
}

TEST_CASE("sign_extend ignores the bits above the field") {
    CHECK(sign_extend(0xabcd'ef7f, 8) == 127);
    CHECK(sign_extend(0xabcd'ef80, 8) == -128);
    CHECK(sign_extend(0xffff'ff05, 4) == 5);
    CHECK(sign_extend(0x0000'0f0f, 4) == -1);
    CHECK(sign_extend(0xfffe, 1) == 0);
}

static std::int32_t pick(std::uint32_t& state) {
    state = state * 1664525 + 1013904223;
    // Mix values near the limits in with ordinary ones.
    switch (state >> 29) {
    case 0:
        return INT32_MAX - static_cast<std::int32_t>((state >> 8) & 0xff);
    case 1:
        return INT32_MIN + static_cast<std::int32_t>((state >> 8) & 0xff);
    case 2:
        return static_cast<std::int32_t>((state >> 8) & 0xffff) - 0x8000;
    default:
        return static_cast<std::int32_t>(state * 2654435761u);
    }
}

TEST_CASE("the add functions agree with 64-bit arithmetic") {
    std::uint32_t state = 7;
    for (int i = 0; i < 4000; ++i) {
        std::int32_t a = pick(state);
        std::int32_t b = pick(state);
        std::int64_t exact = std::int64_t{a} + b;
        bool over = exact > INT32_MAX || exact < INT32_MIN;
        CAPTURE(a);
        CAPTURE(b);
        REQUIRE(add_overflows(a, b) == over);
        REQUIRE(wrapping_add(a, b) == static_cast<std::int32_t>(static_cast<std::uint64_t>(exact)));
        std::int64_t clamped = exact > INT32_MAX   ? INT32_MAX
                               : exact < INT32_MIN ? INT32_MIN
                                                   : exact;
        REQUIRE(saturating_add(a, b) == clamped);
    }
}

TEST_CASE("the limits themselves") {
    CHECK_FALSE(add_overflows(INT32_MAX, 0));
    CHECK_FALSE(add_overflows(INT32_MIN, 0));
    CHECK_FALSE(add_overflows(INT32_MIN, INT32_MAX));
    CHECK(add_overflows(INT32_MIN, INT32_MIN));
    CHECK(add_overflows(INT32_MAX, INT32_MAX));
    CHECK(wrapping_add(INT32_MAX, INT32_MAX) == -2);
    CHECK(wrapping_add(INT32_MIN, INT32_MIN) == 0);
    CHECK(saturating_add(INT32_MIN, INT32_MIN) == INT32_MIN);
    CHECK(saturating_add(INT32_MAX, INT32_MAX) == INT32_MAX);
    CHECK(saturating_add(INT32_MIN, INT32_MAX) == -1);
}

TEST_CASE("checked_mul agrees with 64-bit arithmetic") {
    std::uint32_t state = 99;
    for (int i = 0; i < 4000; ++i) {
        std::int32_t a = pick(state);
        std::int32_t b = (i % 3 == 0) ? pick(state) : static_cast<std::int32_t>(state >> 28) - 8;
        std::int64_t exact = std::int64_t{a} * b;
        CAPTURE(a);
        CAPTURE(b);
        if (exact > INT32_MAX || exact < INT32_MIN) {
            REQUIRE(checked_mul(a, b) == std::nullopt);
        } else {
            REQUIRE(checked_mul(a, b) == static_cast<std::int32_t>(exact));
        }
    }
    CHECK(checked_mul(INT32_MIN, -1) == std::nullopt);
    CHECK(checked_mul(INT32_MIN, 1) == INT32_MIN);
    CHECK(checked_mul(-1, INT32_MAX) == -INT32_MAX);
    CHECK(checked_mul(46341, 46341) == std::nullopt);
    CHECK(checked_mul(46340, 46340) == 2147395600);
    CHECK(checked_mul(-65536, 32768) == INT32_MIN);
    CHECK(checked_mul(0, INT32_MIN) == 0);
}
