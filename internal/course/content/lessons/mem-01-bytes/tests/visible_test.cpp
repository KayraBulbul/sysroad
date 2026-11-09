#include "doctest.h"

#include <bit>
#include <cstdint>

#include "bytes.hpp"

static_assert(std::endian::native == std::endian::little,
              "these tests expect a little-endian machine");

TEST_CASE("hex_bytes") {
    const unsigned char b[] = {0x01, 0xab, 0x00, 0xff};
    CHECK(hex_bytes(b, 4) == "01 ab 00 ff");
    CHECK(hex_bytes(b, 1) == "01");
    CHECK(hex_bytes(b, 0) == "");
}

TEST_CASE("an int is stored little end first") {
    CHECK(bytes_of(1) == "01 00 00 00");
    CHECK(bytes_of(std::uint32_t{0x01020304}) == "04 03 02 01");
}

TEST_CASE("bits_of") {
    CHECK(bits_of(1.0f) == 0x3f800000);
    CHECK(bits_of(-2.0f) == 0xc0000000);
}

TEST_CASE("parts_of 1.0") {
    FloatParts p = parts_of(1.0f);
    CHECK(p.sign == 0);
    CHECK(p.exponent == 127);
    CHECK(p.mantissa == 0);
}
