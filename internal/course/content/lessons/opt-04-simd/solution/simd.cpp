#include "simd.hpp"

bool is_ascii(const char* s, std::size_t n) {
    // No early exit: OR every byte together and look at the top bit once.
    unsigned char seen = 0;
    for (std::size_t i = 0; i < n; ++i) {
        seen |= static_cast<unsigned char>(s[i]);
    }
    return seen < 128;
}

std::size_t count_byte(const char* s, std::size_t n, char c) {
    const u8x16 needle = splat16(static_cast<std::uint8_t>(c));
    std::size_t total = 0;
    std::size_t i = 0;
    while (i + 16 <= n) {
        // A lane can count to 255, so empty the counters every 255 blocks.
        u8x16 counts{};
        for (int block = 0; block < 255 && i + 16 <= n; ++block, i += 16) {
            // A match is 0xff in its lane, which is -1: subtracting it adds 1.
            counts -= reinterpret_cast<u8x16>(load16(s + i) == needle);
        }
        for (int lane = 0; lane < 16; ++lane) {
            total += counts[lane];
        }
    }
    for (; i < n; ++i) {
        total += s[i] == c;
    }
    return total;
}
