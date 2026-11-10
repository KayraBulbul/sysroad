#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <optional>

// The low `bits` bits of x, read as a two's complement number. Higher bits
// of x are ignored. sign_extend(200, 8) is -56 and sign_extend(200, 9) is 200.
// 1 <= bits <= 32.
std::int32_t sign_extend(std::uint32_t x, int bits);

// Whether the exact sum a + b is outside the range of a 32-bit int.
bool add_overflows(std::int32_t a, std::int32_t b);

// a + b the way the CPU adds: the low 32 bits of the exact sum.
std::int32_t wrapping_add(std::int32_t a, std::int32_t b);

// a + b, or INT32_MAX / INT32_MIN if the exact sum is beyond them.
std::int32_t saturating_add(std::int32_t a, std::int32_t b);

// a * b, or nullopt if the exact product doesn't fit in 32 bits.
std::optional<std::int32_t> checked_mul(std::int32_t a, std::int32_t b);
