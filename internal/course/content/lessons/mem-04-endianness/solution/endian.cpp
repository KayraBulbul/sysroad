#include "endian.hpp"

std::uint16_t load_le16(const unsigned char* p) {
    return std::uint16_t(p[0] | p[1] << 8);
}

std::uint32_t load_le32(const unsigned char* p) {
    return std::uint32_t(p[0]) | std::uint32_t(p[1]) << 8 | std::uint32_t(p[2]) << 16 |
           std::uint32_t(p[3]) << 24;
}

std::uint16_t load_be16(const unsigned char* p) {
    return std::uint16_t(p[0] << 8 | p[1]);
}

std::uint32_t load_be32(const unsigned char* p) {
    return std::uint32_t(p[0]) << 24 | std::uint32_t(p[1]) << 16 | std::uint32_t(p[2]) << 8 |
           std::uint32_t(p[3]);
}
