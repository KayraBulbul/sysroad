#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <vector>

// (a + b) / 2, rounded towards zero, for any two ints.
int midpoint(int a, int b);

// x rotated left by n bits: bits shifted off the top come back at the
// bottom. Any n >= 0 works; rotating by 32 is the same as by 0.
std::uint32_t rotl(std::uint32_t x, unsigned n);

// The 4 bytes at p as a uint32_t, in the machine's byte order. p can be any address.
std::uint32_t read_u32(const unsigned char* p);

// Whether any two neighbouring elements are equal.
bool has_adjacent_equal(const std::vector<int>& v);

// |x| for any int, including INT_MIN.
unsigned magnitude(int x);
