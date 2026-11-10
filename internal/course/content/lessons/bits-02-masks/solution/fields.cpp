#include "fields.hpp"

static std::uint32_t mask_of(int width) {
    return width >= 32 ? 0xffffffffu : (1u << width) - 1;
}

std::uint32_t get_field(std::uint32_t x, int lo, int width) {
    return (x >> lo) & mask_of(width);
}

std::uint32_t set_field(std::uint32_t x, int lo, int width, std::uint32_t value) {
    std::uint32_t mask = mask_of(width) << lo;
    return (x & ~mask) | ((value << lo) & mask);
}

Rgb unpack565(std::uint16_t pixel) {
    std::uint32_t r = get_field(pixel, 11, 5);
    std::uint32_t g = get_field(pixel, 5, 6);
    std::uint32_t b = get_field(pixel, 0, 5);
    Rgb out;
    out.r = static_cast<std::uint8_t>((r << 3) | (r >> 2));
    out.g = static_cast<std::uint8_t>((g << 2) | (g >> 4));
    out.b = static_cast<std::uint8_t>((b << 3) | (b >> 2));
    return out;
}

std::uint16_t pack565(Rgb colour) {
    return static_cast<std::uint16_t>((colour.r >> 3) << 11 | (colour.g >> 2) << 5 |
                                      (colour.b >> 3));
}

std::string mode_string(unsigned mode) {
    std::string out;
    for (int i = 8; i >= 0; --i) {
        out += mode & (1u << i) ? "rwxrwxrwx"[8 - i] : '-';
    }
    return out;
}
