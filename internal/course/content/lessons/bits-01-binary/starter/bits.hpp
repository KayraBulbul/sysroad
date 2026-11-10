#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// All 32 bits of x, highest first, in groups of four with one space between:
// "0000 0000 0000 0000 0000 0000 1100 1010" for 202.
std::string to_binary(std::uint32_t x);

// The number written in s: decimal ("202"), hex after 0x ("0xCA", "0xca") or
// binary after 0b ("0b11001010"). nullopt when s is empty, has no digits, has
// a character that isn't a digit of that base, or doesn't fit in 32 bits.
std::optional<std::uint32_t> parse_number(std::string_view s);

// How many bits of x are 1.
int count_ones(std::uint32_t x);

// Whether x is 1, 2, 4, 8, ...
bool is_power_of_two(std::uint32_t x);
