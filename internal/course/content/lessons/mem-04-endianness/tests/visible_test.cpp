#include "doctest.h"

#include <string>

#include "endian.hpp"
#include "png.hpp"

static const unsigned char word[] = {0xde, 0xad, 0xbe, 0xef};

TEST_CASE("big-endian") {
    CHECK(load_be32(word) == 0xdeadbeef);
    CHECK(load_be16(word) == 0xdead);
}

TEST_CASE("little-endian") {
    CHECK(load_le32(word) == 0xefbeadde);
    CHECK(load_le16(word) == 0xadde);
}

// The first 33 bytes of a 64 x 48 RGBA image.
static const std::string logo("\x89PNG\r\n\x1a\n"
                              "\0\0\0\x0dIHDR"
                              "\0\0\0\x40\0\0\0\x30\x08\x06\0\0\0"
                              "\x12\x34\x56\x78",
                              33);

TEST_CASE("parse_png reads the IHDR fields") {
    auto info = parse_png(logo);
    REQUIRE(info.has_value());
    CHECK(info->width == 64);
    CHECK(info->height == 48);
    CHECK(info->bit_depth == 8);
    CHECK(info->color_type == 6);
    CHECK_FALSE(info->interlaced);
    CHECK(describe(*info) == "64 x 48, 8-bit RGBA");
}

TEST_CASE("not a PNG") {
    CHECK_FALSE(parse_png("hello, this is plain text and long enough").has_value());
    CHECK_FALSE(parse_png("").has_value());
}
