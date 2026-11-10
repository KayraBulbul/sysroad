#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <ostream>
#include <string>

// The width bits of x that start at bit lo, moved down to bit 0.
// get_field(0xabcd, 4, 8) is 0xbc. 1 <= width <= 32 and lo + width <= 32.
std::uint32_t get_field(std::uint32_t x, int lo, int width);

// x with that field replaced by value. Bits of value that don't fit in
// width bits are dropped, and the rest of x is unchanged.
std::uint32_t set_field(std::uint32_t x, int lo, int width, std::uint32_t value);

struct Rgb {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;

    bool operator==(const Rgb&) const = default;
};

// So a failing test can show the colour.
inline std::ostream& operator<<(std::ostream& os, const Rgb& c) {
    return os << "{" << int(c.r) << ", " << int(c.g) << ", " << int(c.b) << "}";
}

// An RGB565 pixel (red in bits 15-11, green in 10-5, blue in 4-0) as 8-bit
// channels. Each channel is widened by copying its top bits into the low
// bits, so 31 becomes 255 and 0 stays 0.
Rgb unpack565(std::uint16_t pixel);

// The nearest-below RGB565 pixel: each channel keeps its top 5 or 6 bits.
std::uint16_t pack565(Rgb colour);

// The low nine bits of mode the way ls -l shows them: 0754 is "rwxr-xr--".
std::string mode_string(unsigned mode);
