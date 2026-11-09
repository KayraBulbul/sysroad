#include "doctest.h"

#include <cstdint>
#include <limits>

#include "bytes.hpp"

TEST_CASE("negative numbers are two's complement") {
    CHECK(bytes_of(-1) == "ff ff ff ff");
    CHECK(bytes_of(std::int16_t{-2}) == "fe ff");
}

TEST_CASE("a double is 8 bytes") {
    CHECK(bytes_of(1.5) == "00 00 00 00 00 00 f8 3f");
}

TEST_CASE("parts_of: the special cases") {
    FloatParts z = parts_of(-0.0f);
    CHECK(z.sign == 1);
    CHECK(z.exponent == 0);
    CHECK(z.mantissa == 0);

    FloatParts h = parts_of(-0.75f);  // -1.5 × 2^-1
    CHECK(h.sign == 1);
    CHECK(h.exponent == 126);
    CHECK(h.mantissa == 0x400000);

    FloatParts inf = parts_of(std::numeric_limits<float>::infinity());
    CHECK(inf.exponent == 255);
    CHECK(inf.mantissa == 0);

    FloatParts nan = parts_of(std::numeric_limits<float>::quiet_NaN());
    CHECK(nan.exponent == 255);
    CHECK(nan.mantissa != 0);

    FloatParts tiny = parts_of(std::numeric_limits<float>::denorm_min());
    CHECK(tiny.exponent == 0);
    CHECK(tiny.mantissa == 1);
}

TEST_CASE("parts_of 0.1 is rounded") {
    FloatParts p = parts_of(0.1f);
    CHECK(p.exponent == 123);  // 1.6 × 2^-4
    CHECK(p.mantissa == 0x4ccccd);
}
