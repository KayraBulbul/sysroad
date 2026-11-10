#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>

enum class FloatClass { Zero, Subnormal, Normal, Infinity, NaN };

// Which kind of float these 32 bits are.
FloatClass classify(std::uint32_t bits);

// The number these 32 bits stand for, worked out from the sign, exponent
// and fraction fields. A NaN pattern gives NaN, and -0 gives -0.0.
double value_of(std::uint32_t bits);

// The bits of the float nearest to n. When n is exactly halfway between
// two floats, the one with an even fraction wins.
std::uint32_t from_uint(std::uint32_t n);
