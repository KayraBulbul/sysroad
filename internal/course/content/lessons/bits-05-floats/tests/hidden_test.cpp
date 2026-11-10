#include "doctest.h"

#include <bit>
#include <cmath>
#include <cstdint>

#include "ieee.hpp"

static FloatClass reference_class(float f) {
    switch (std::fpclassify(f)) {
    case FP_ZERO:
        return FloatClass::Zero;
    case FP_SUBNORMAL:
        return FloatClass::Subnormal;
    case FP_INFINITE:
        return FloatClass::Infinity;
    case FP_NAN:
        return FloatClass::NaN;
    default:
        return FloatClass::Normal;
    }
}

static void check_bits(std::uint32_t bits) {
    float f = std::bit_cast<float>(bits);
    CAPTURE(bits);
    REQUIRE(classify(bits) == reference_class(f));
    double v = value_of(bits);
    if (std::isnan(f)) {
        REQUIRE(std::isnan(v));
    } else {
        REQUIRE(v == static_cast<double>(f));
        REQUIRE(std::signbit(v) == std::signbit(f));
    }
}

TEST_CASE("classify and value_of agree with the CPU on many bit patterns") {
    std::uint32_t x = 0x12345678;
    for (int i = 0; i < 20000; ++i) {
        x = x * 1664525 + 1013904223;
        check_bits(x);
    }
}

TEST_CASE("the patterns around every boundary") {
    for (std::uint32_t sign : {0u, 0x80000000u}) {
        for (std::uint32_t exponent : {0u, 1u, 2u, 126u, 127u, 128u, 150u, 253u, 254u, 255u}) {
            for (std::uint32_t fraction : {0u, 1u, 2u, 0x400000u, 0x7ffffeu, 0x7fffffu}) {
                check_bits(sign | exponent << 23 | fraction);
            }
        }
    }
}

TEST_CASE("negative zero and negative infinity keep their sign") {
    CHECK(std::signbit(value_of(0x80000000)));
    CHECK_FALSE(std::signbit(value_of(0x00000000)));
    CHECK(value_of(0xff800000) < 0);
    CHECK(std::isinf(value_of(0xff800000)));
    CHECK(value_of(0x7f800000) > 0);
}

static void check_int(std::uint32_t n) {
    CAPTURE(n);
    REQUIRE(from_uint(n) == std::bit_cast<std::uint32_t>(static_cast<float>(n)));
}

TEST_CASE("from_uint agrees with the CPU on small numbers") {
    for (std::uint32_t n = 0; n < 5000; ++n) {
        check_int(n);
    }
}

TEST_CASE("from_uint agrees with the CPU around every power of two") {
    for (int shift = 0; shift < 32; ++shift) {
        std::uint32_t p = 1u << shift;
        for (std::uint32_t d = 0; d < 300; ++d) {
            check_int(p + d);
            check_int(p - d);
        }
    }
    for (std::uint32_t d = 0; d < 1000; ++d) {
        check_int(0xffffffffu - d);
    }
}

TEST_CASE("from_uint agrees with the CPU on many numbers") {
    std::uint32_t x = 0xcafef00d;
    for (int i = 0; i < 50000; ++i) {
        x = x * 1664525 + 1013904223;
        check_int(x);
        check_int(x >> (x & 15));
    }
}

TEST_CASE("from_uint: halfway cases go to the even neighbour") {
    // Between 2^25 and 2^26 floats are 4 apart, so +2 is exactly halfway.
    CHECK(from_uint(0x02000002) == 0x4c000000);
    CHECK(from_uint(0x02000006) == 0x4c000002);
    CHECK(from_uint(0x02000003) == 0x4c000001);
    CHECK(from_uint(0x02000001) == 0x4c000000);
    // All ones in the kept part, rounding up into the next exponent.
    CHECK(from_uint(0x01ffffff) == 0x4c000000);
    CHECK(from_uint(0xffffff80) == 0x4f800000);
    CHECK(from_uint(0xffffff7f) == 0x4f7fffff);
}
