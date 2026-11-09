#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <span>
#include <stdexcept>

// What's wrong with the file. parse_bmp throws exactly these messages:
//   "not a BMP file"              no "BM" at the start
//   "truncated header"            the file ends inside the 54 header bytes
//   "unsupported header size N"   the info header is smaller than 40 bytes
//   "bad size W x H"              width <= 0, height == 0 or INT32_MIN
//   "unsupported: N bits per pixel"   anything but 24 and 32
//   "unsupported compression N"   anything but 0, or 3 at 32 bits
//   "pixel data is truncated"     the rows don't fit in the file
// and pixel_at throws "(X, Y) is outside the W x H image".
struct BmpError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct BmpInfo {
    std::int32_t width = 0;
    std::int32_t height = 0;  // always positive
    bool top_down = false;    // the file stored a negative height
    int bits = 0;             // 24 or 32
    std::uint32_t pixel_offset = 0;
    std::uint32_t row_size = 0;  // bytes per row in the file, padding included
};

struct Rgb {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    bool operator==(const Rgb&) const = default;
};

// Reads and checks a BMP file's headers.
BmpInfo parse_bmp(std::span<const unsigned char> file);

// The colour at (x, y), where (0, 0) is the top-left corner.
Rgb pixel_at(std::span<const unsigned char> file, const BmpInfo& info, int x, int y);
