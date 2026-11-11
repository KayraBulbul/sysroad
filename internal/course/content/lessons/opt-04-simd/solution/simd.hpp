#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstddef>
#include <cstdint>
#include <cstring>

// 16 bytes in one SIMD register: SSE on x86-64, NEON on arm64. This is a
// clang and GCC extension. Arithmetic and comparisons work on all 16 lanes
// at once, and v[i] reads lane i.
using u8x16 = std::uint8_t __attribute__((vector_size(16)));

// The 16 bytes at p, which needn't be aligned.
inline u8x16 load16(const char* p) {
    u8x16 v;
    std::memcpy(&v, p, sizeof v);
    return v;
}

// A vector with b in every lane.
inline u8x16 splat16(std::uint8_t b) {
    return u8x16{} + b;
}

// Whether every byte of s[0, n) is below 128, that is, plain ASCII.
bool is_ascii(const char* s, std::size_t n);

// How many bytes of s[0, n) equal c. Uses u8x16.
std::size_t count_byte(const char* s, std::size_t n, char c);
