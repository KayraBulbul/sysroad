#include "doctest.h"

#include <cstdint>

#include "fields.hpp"

TEST_CASE("a field can be the whole word, or its top bits") {
    CHECK(get_field(0xdeadbeef, 0, 32) == 0xdeadbeef);
    CHECK(get_field(0xdeadbeef, 31, 1) == 1);
    CHECK(get_field(0xdeadbeef, 16, 16) == 0xdead);
    CHECK(get_field(0xdeadbeef, 1, 31) == 0xdeadbeef >> 1);
    CHECK(set_field(0xdeadbeef, 0, 32, 0x12345678) == 0x12345678);
    CHECK(set_field(0x7fffffff, 31, 1, 1) == 0xffffffff);
    CHECK(set_field(0xdeadbeef, 16, 16, 0xcafe) == 0xcafebeef);
}

TEST_CASE("set_field drops the bits of value that don't fit") {
    CHECK(set_field(0, 4, 4, 0xff) == 0xf0);
    CHECK(set_field(0xffffffff, 4, 4, 0x10) == 0xffffff0f);
    CHECK(set_field(0x0000ffff, 12, 8, 0xabc) == 0x000bcfff);
}

TEST_CASE("get_field undoes set_field everywhere") {
    std::uint32_t x = 0x243f6a88;
    for (int width = 1; width <= 32; ++width) {
        for (int lo = 0; lo + width <= 32; ++lo) {
            x = x * 1664525 + 1013904223;
            std::uint32_t value = x >> 7;
            std::uint32_t y = set_field(x, lo, width, value);
            std::uint32_t mask = width == 32 ? 0xffffffffu : ((1u << width) - 1);
            CAPTURE(width);
            CAPTURE(lo);
            REQUIRE(get_field(y, lo, width) == (value & mask));
            // Everything outside the field is as it was.
            REQUIRE((y & ~(mask << lo)) == (x & ~(mask << lo)));
        }
    }
}

TEST_CASE("every 565 pixel survives a round trip") {
    for (std::uint32_t p = 0; p <= 0xffff; ++p) {
        auto pixel = static_cast<std::uint16_t>(p);
        if (pack565(unpack565(pixel)) != pixel) {
            CAPTURE(p);
            FAIL("pack565(unpack565(pixel)) changed the pixel");
        }
    }
}

TEST_CASE("channels are widened by repeating their top bits") {
    CHECK(unpack565(0xf800) == Rgb{255, 0, 0});
    CHECK(unpack565(0x07e0) == Rgb{0, 255, 0});
    CHECK(unpack565(0x001f) == Rgb{0, 0, 255});
    CHECK(unpack565(0x8410) == Rgb{132, 130, 132});
    CHECK(unpack565(0x0841) == Rgb{8, 8, 8});
    CHECK(pack565(Rgb{7, 3, 7}) == 0);
    CHECK(pack565(Rgb{8, 8, 8}) == 0x0841);
    CHECK(pack565(Rgb{15, 7, 15}) == 0x0821);
    CHECK(pack565(Rgb{0x12, 0x34, 0x56}) == 0x11aa);
}

TEST_CASE("mode_string: every bit and no others") {
    CHECK(mode_string(0) == "---------");
    CHECK(mode_string(0777) == "rwxrwxrwx");
    CHECK(mode_string(0421) == "r---w---x");
    CHECK(mode_string(0124) == "--x-w-r--");
    CHECK(mode_string(0100) == "--x------");
    // Bits above the nine are someone else's business.
    CHECK(mode_string(0100644) == "rw-r--r--");
    CHECK(mode_string(04755) == "rwxr-xr-x");
}
