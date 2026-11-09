#include "bytes.hpp"

#include <bit>
#include <cstring>
#include <format>

std::string hex_bytes(const void* p, std::size_t n) {
    auto* b = static_cast<const unsigned char*>(p);
    std::string out;
    for (std::size_t i = 0; i < n; ++i) {
        if (i > 0) {
            out += ' ';
        }
        out += std::format("{:02x}", b[i]);
    }
    return out;
}

std::uint32_t bits_of(float f) {
    return std::bit_cast<std::uint32_t>(f);
}

FloatParts parts_of(float f) {
    std::uint32_t u = bits_of(f);
    return {u >> 31, (u >> 23) & 0xff, u & 0x7fffff};
}
