#include "simd.hpp"

bool is_ascii(const char* s, std::size_t n) {
    // Correct, but clang won't vectorise a loop that can stop early.
    for (std::size_t i = 0; i < n; ++i) {
        if (static_cast<unsigned char>(s[i]) >= 128) {
            return false;
        }
    }
    return true;
}

std::size_t count_byte(const char* s, std::size_t n, char c) {
    // TODO: 16 bytes at a time with u8x16.
    (void)s;
    (void)n;
    (void)c;
    return 0;
}
