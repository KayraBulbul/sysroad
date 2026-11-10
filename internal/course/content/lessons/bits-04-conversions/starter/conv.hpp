#pragma once
// What each function promises. You won't need to change this file.

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

// Whether x is less than limit, as numbers: below(-1, 5) is true.
bool below(int x, unsigned limit);

// The index of the last element equal to x, or -1 if there is none.
int last_index_of(const std::vector<int>& v, int x);

// How many times each byte value, 0 to 255, appears in s.
std::array<int, 256> byte_histogram(std::string_view s);

// The average of two bytes, rounded down: average(200, 100) is 150.
std::uint8_t average(std::uint8_t a, std::uint8_t b);

// A size in mebibytes as bytes: mib * 1024 * 1024, for any mib.
std::uint64_t mib_to_bytes(std::uint32_t mib);

// A value with only bit n set. 0 <= n <= 63.
std::uint64_t bit(int n);
