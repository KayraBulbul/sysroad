#pragma once
// The course's readers from lesson 5.3.5. You won't need to change this file.

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

// n little-endian bytes at `at` in b. Throws std::out_of_range past the end.
inline std::uint64_t read_le(std::string_view b, std::size_t at, std::size_t n) {
    if (at > b.size() || b.size() - at < n) {
        throw std::out_of_range("reading " + std::to_string(n) + " bytes at " + std::to_string(at) +
                                " of " + std::to_string(b.size()));
    }
    std::uint64_t v = 0;
    for (std::size_t i = 0; i < n; ++i) {
        v |= std::uint64_t{static_cast<unsigned char>(b[at + i])} << (8 * i);
    }
    return v;
}

inline std::uint16_t read_u16(std::string_view b, std::size_t at) {
    return static_cast<std::uint16_t>(read_le(b, at, 2));
}
inline std::uint32_t read_u32(std::string_view b, std::size_t at) {
    return static_cast<std::uint32_t>(read_le(b, at, 4));
}
inline std::uint64_t read_u64(std::string_view b, std::size_t at) {
    return read_le(b, at, 8);
}
