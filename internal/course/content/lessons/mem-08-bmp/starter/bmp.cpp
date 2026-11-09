#include "bmp.hpp"

#include <format>

#include "reader.hpp"

BmpInfo parse_bmp(std::span<const unsigned char> file) {
    // TODO: read the headers with a ByteReader, check them, fill in BmpInfo.
    (void)file;
    throw BmpError("not a BMP file");
}

Rgb pixel_at(std::span<const unsigned char> file, const BmpInfo& info, int x, int y) {
    // TODO
    (void)file;
    (void)info;
    (void)x;
    (void)y;
    return {};
}
