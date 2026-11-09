#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <cstdint>
#include <string>

// n bytes starting at p, in memory order: "01 00 00 00".
std::string hex_bytes(const void* p, std::size_t n);

// The bytes of any object.
template <typename T> std::string bytes_of(const T& value) {
    return hex_bytes(&value, sizeof value);
}

// A float's 32 bits, unchanged.
std::uint32_t bits_of(float f);

// The three fields of an IEEE 754 single-precision float.
struct FloatParts {
    std::uint32_t sign;      // 1 bit
    std::uint32_t exponent;  // 8 bits, stored as the real exponent + 127
    std::uint32_t mantissa;  // 23 bits, without the leading 1
};

FloatParts parts_of(float f);
