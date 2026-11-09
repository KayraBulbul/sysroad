#include "doctest.h"

#include <string>

#include "endian.hpp"
#include "png.hpp"

TEST_CASE("every byte lands in the right place") {
    const unsigned char b[] = {0x01, 0x02, 0x03, 0x04, 0xff, 0x80};
    CHECK(load_be32(b) == 0x01020304);
    CHECK(load_le32(b) == 0x04030201);
    CHECK(load_be32(b + 2) == 0x0304ff80);
    CHECK(load_le32(b + 2) == 0x80ff0403);
    CHECK(load_be16(b + 4) == 0xff80);
    CHECK(load_le16(b + 4) == 0x80ff);
}

static std::string header(const std::string& ihdr_data, const char* type = "IHDR", char len = 13) {
    return std::string("\x89PNG\r\n\x1a\n", 8) + std::string("\0\0\0", 3) + len + type + ihdr_data +
           std::string(4, '\0');
}

TEST_CASE("a wide interlaced grey + alpha image") {
    auto info =
        parse_png(header(std::string("\x00\x01\x11\x70\x00\x00\x00\x03\x10\x04\0\0\x01", 13)));
    REQUIRE(info.has_value());
    CHECK(info->width == 70000);
    CHECK(info->height == 3);
    CHECK(describe(*info) == "70000 x 3, 16-bit grey + alpha, interlaced");
}

TEST_CASE("the other colour types") {
    PngInfo i;
    i.width = 1;
    i.height = 2;
    i.bit_depth = 1;
    CHECK(describe(i) == "1 x 2, 1-bit grey");
    i.color_type = 2;
    CHECK(describe(i) == "1 x 2, 1-bit RGB");
    i.color_type = 3;
    CHECK(describe(i) == "1 x 2, 1-bit palette");
    i.color_type = 5;
    CHECK(describe(i) == "1 x 2, 1-bit unknown");
}

TEST_CASE("broken headers are rejected") {
    std::string ok = header(std::string("\0\0\0\x01\0\0\0\x01\x08\x02\0\0\0", 13));
    REQUIRE(parse_png(ok).has_value());
    // Too short to hold the fields: must not read past the end.
    CHECK_FALSE(parse_png(std::string_view(ok).substr(0, 20)).has_value());
    CHECK_FALSE(parse_png(std::string_view(ok).substr(0, 8)).has_value());
    // Wrong chunk type or length.
    CHECK_FALSE(parse_png(header(std::string(13, '\0'), "IDAT")).has_value());
    CHECK_FALSE(parse_png(header(std::string(13, '\0'), "IHDR", 12)).has_value());
    // A signature with one byte changed.
    std::string bad = ok;
    bad[0] = 'P';
    CHECK_FALSE(parse_png(bad).has_value());
    bad = ok;
    bad[4] = '\n';
    CHECK_FALSE(parse_png(bad).has_value());
}
