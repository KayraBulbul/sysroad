#pragma once
// Builds BMP files in memory for the tests.

#include <cstdint>
#include <vector>

struct BmpSpec {
    std::int32_t width = 2;
    std::int32_t height = 2;  // negative: top-down
    int bits = 24;
    std::uint32_t compression = 0;
    std::uint32_t dib = 40;
    std::uint32_t extra = 0;  // bytes between the headers and the pixels
};

inline void put16(std::vector<unsigned char>& v, std::uint32_t x) {
    v.push_back(x & 0xff);
    v.push_back((x >> 8) & 0xff);
}

inline void put32(std::vector<unsigned char>& v, std::uint32_t x) {
    put16(v, x & 0xffff);
    put16(v, x >> 16);
}

// Pixel (x, y) in file order has colour r = 10 * x + y, g = 100 + row, b = 200 + x,
// where row counts rows as stored in the file.
inline std::vector<unsigned char> make_bmp(const BmpSpec& s) {
    std::vector<unsigned char> v = {'B', 'M'};
    std::int64_t h = s.height < 0 ? -std::int64_t(s.height) : s.height;
    std::uint32_t row = (std::uint32_t(s.bits) * s.width + 31) / 32 * 4;
    std::uint32_t offset = 14 + s.dib + s.extra;
    put32(v, offset + row * h);
    put32(v, 0);
    put32(v, offset);
    put32(v, s.dib);
    put32(v, std::uint32_t(s.width));
    put32(v, std::uint32_t(s.height));
    put16(v, 1);
    put16(v, s.bits);
    put32(v, s.compression);
    while (v.size() < offset) {
        v.push_back(0);
    }
    for (std::int64_t r = 0; r < h; ++r) {
        std::size_t start = v.size();
        for (std::int32_t x = 0; x < s.width; ++x) {
            v.push_back(200 + x);     // b
            v.push_back(100 + r);     // g
            v.push_back(10 * x + r);  // r
            if (s.bits == 32) {
                v.push_back(255);
            }
        }
        while (v.size() - start < row) {
            v.push_back(0xee);  // padding
        }
    }
    return v;
}
