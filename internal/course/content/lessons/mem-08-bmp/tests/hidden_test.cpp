#include "doctest.h"

#include <cstdint>

#include "bmp.hpp"
#include "bmp_maker.hpp"

TEST_CASE("a top-down 32-bit image with bitfields") {
    auto file = make_bmp({.width = 3, .height = -2, .bits = 32, .compression = 3});
    BmpInfo info = parse_bmp(file);
    CHECK(info.height == 2);
    CHECK(info.top_down);
    CHECK(info.row_size == 12);
    CHECK(pixel_at(file, info, 0, 0) == Rgb{0, 100, 200});
    CHECK(pixel_at(file, info, 2, 1) == Rgb{21, 101, 202});
}

TEST_CASE("a newer, longer info header moves the pixels") {
    auto file = make_bmp({.width = 1, .height = 1, .dib = 124, .extra = 6});
    BmpInfo info = parse_bmp(file);
    CHECK(info.pixel_offset == 14 + 124 + 6);
    CHECK(info.row_size == 4);
    CHECK(pixel_at(file, info, 0, 0) == Rgb{0, 100, 200});
}

TEST_CASE("the header errors") {
    std::vector<unsigned char> bm = {'B', 'M'};
    CHECK_THROWS_WITH_AS(parse_bmp(bm), "truncated header", BmpError);
    std::vector<unsigned char> b = {'B'};
    CHECK_THROWS_WITH_AS(parse_bmp(b), "not a BMP file", BmpError);
    CHECK_THROWS_WITH_AS(parse_bmp(std::vector<unsigned char>{}), "not a BMP file", BmpError);

    auto file = make_bmp({});
    file.resize(53);
    CHECK_THROWS_WITH_AS(parse_bmp(file), "truncated header", BmpError);

    auto core = make_bmp({.dib = 12});
    core.resize(60, 0);
    CHECK_THROWS_WITH_AS(parse_bmp(core), "unsupported header size 12", BmpError);
}

TEST_CASE("bad sizes") {
    CHECK_THROWS_WITH_AS(parse_bmp(make_bmp({.width = 0})), "bad size 0 x 2", BmpError);
    CHECK_THROWS_WITH_AS(parse_bmp(make_bmp({.height = 0})), "bad size 2 x 0", BmpError);
    auto file = make_bmp({.width = 1, .height = 1});
    file[22] = 0x00;  // height = INT32_MIN: -height doesn't fit
    file[23] = 0x00;
    file[24] = 0x00;
    file[25] = 0x80;
    CHECK_THROWS_WITH_AS(parse_bmp(file), "bad size 1 x -2147483648", BmpError);
    file[18] = 0xff;  // width = -1
    file[19] = 0xff;
    file[20] = 0xff;
    file[21] = 0xff;
    CHECK_THROWS_AS(parse_bmp(file), BmpError);
}

TEST_CASE("unsupported formats") {
    CHECK_THROWS_WITH_AS(parse_bmp(make_bmp({.bits = 8})), "unsupported: 8 bits per pixel",
                         BmpError);
    CHECK_THROWS_WITH_AS(parse_bmp(make_bmp({.bits = 16})), "unsupported: 16 bits per pixel",
                         BmpError);
    CHECK_THROWS_WITH_AS(parse_bmp(make_bmp({.compression = 1})), "unsupported compression 1",
                         BmpError);
    CHECK_THROWS_WITH_AS(parse_bmp(make_bmp({.compression = 3})), "unsupported compression 3",
                         BmpError);
}

TEST_CASE("the pixels must fit in the file") {
    auto file = make_bmp({.width = 5, .height = 3});
    CHECK_NOTHROW(parse_bmp(file));
    file.pop_back();  // one byte of padding short
    CHECK_THROWS_WITH_AS(parse_bmp(file), "pixel data is truncated", BmpError);
}

TEST_CASE("a giant claimed size doesn't overflow the check") {
    auto file = make_bmp({.width = 1, .height = 1});
    // width = 0x7fffffff: row_size * height is far beyond 4 GB.
    file[18] = 0xff;
    file[19] = 0xff;
    file[20] = 0xff;
    file[21] = 0x7f;
    file[22] = 0xff;  // height = 0x7fff00ff
    file[24] = 0xff;
    file[25] = 0x7f;
    CHECK_THROWS_WITH_AS(parse_bmp(file), "pixel data is truncated", BmpError);
}

TEST_CASE("pixel_at outside the image") {
    auto file = make_bmp({.width = 5, .height = 3});
    BmpInfo info = parse_bmp(file);
    CHECK_THROWS_WITH_AS(pixel_at(file, info, 5, 0), "(5, 0) is outside the 5 x 3 image", BmpError);
    CHECK_THROWS_WITH_AS(pixel_at(file, info, 0, 3), "(0, 3) is outside the 5 x 3 image", BmpError);
    CHECK_THROWS_WITH_AS(pixel_at(file, info, -1, 0), "(-1, 0) is outside the 5 x 3 image",
                         BmpError);
    CHECK_THROWS_AS(pixel_at(file, info, 0, -1), BmpError);
}
