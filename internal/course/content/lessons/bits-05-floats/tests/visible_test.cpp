#include "doctest.h"

#include <cmath>

#include "ieee.hpp"

TEST_CASE("classify") {
    CHECK(classify(0x40d00000) == FloatClass::Normal);     // 6.5
    CHECK(classify(0x00000000) == FloatClass::Zero);       // +0
    CHECK(classify(0x80000000) == FloatClass::Zero);       // -0
    CHECK(classify(0x00000001) == FloatClass::Subnormal);  // the smallest positive float
    CHECK(classify(0x7f800000) == FloatClass::Infinity);
    CHECK(classify(0x7fc00000) == FloatClass::NaN);
}

TEST_CASE("value_of normal numbers") {
    CHECK(value_of(0x40d00000) == 6.5);
    CHECK(value_of(0x3f800000) == 1.0);
    CHECK(value_of(0xbf000000) == -0.5);
    CHECK(value_of(0x3dcccccd) == 0.100000001490116119384765625);
}

TEST_CASE("value_of the special cases") {
    CHECK(value_of(0x00000000) == 0.0);
    CHECK(value_of(0x00000001) == std::ldexp(1.0, -149));
    CHECK(std::isinf(value_of(0x7f800000)));
    CHECK(std::isnan(value_of(0x7fc00000)));
}

TEST_CASE("from_uint: numbers that fit exactly") {
    CHECK(from_uint(0) == 0x00000000);
    CHECK(from_uint(1) == 0x3f800000);
    CHECK(from_uint(6) == 0x40c00000);
    CHECK(from_uint(16777216) == 0x4b800000);
}

TEST_CASE("from_uint: numbers that need rounding") {
    CHECK(from_uint(16777217) == 0x4b800000);    // halfway: down to the even one
    CHECK(from_uint(16777219) == 0x4b800002);    // halfway: up to the even one
    CHECK(from_uint(4294967295) == 0x4f800000);  // rounds up to 2^32
}
