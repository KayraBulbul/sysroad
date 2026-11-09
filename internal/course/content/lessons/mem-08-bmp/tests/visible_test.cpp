#include "doctest.h"

#include "bmp.hpp"
#include "bmp_maker.hpp"

TEST_CASE("a 5 x 3 bottom-up image") {
    auto file = make_bmp({.width = 5, .height = 3});
    BmpInfo info = parse_bmp(file);
    CHECK(info.width == 5);
    CHECK(info.height == 3);
    CHECK_FALSE(info.top_down);
    CHECK(info.bits == 24);
    CHECK(info.pixel_offset == 54);
    CHECK(info.row_size == 16);  // 15 bytes of pixels + 1 of padding
}

TEST_CASE("pixel_at flips bottom-up rows") {
    auto file = make_bmp({.width = 5, .height = 3});
    BmpInfo info = parse_bmp(file);
    // The top row of the image is the last row in the file (row 2).
    CHECK(pixel_at(file, info, 0, 0) == Rgb{2, 102, 200});
    CHECK(pixel_at(file, info, 4, 0) == Rgb{42, 102, 204});
    CHECK(pixel_at(file, info, 1, 2) == Rgb{10, 100, 201});
}

TEST_CASE("not a BMP") {
    std::vector<unsigned char> text = {'h', 'e', 'l', 'l', 'o'};
    CHECK_THROWS_WITH_AS(parse_bmp(text), "not a BMP file", BmpError);
}
