#pragma once
// What each function promises. You won't need to change this file.

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

// Whether a and b are equal for practical purposes: they differ by no more
// than 1e-9 times the largest of |a|, |b| and 1. An infinity is only close
// to itself, and NaN is close to nothing.
bool close_enough(double a, double b);

// The reading, in seconds, of a clock that ticks ten times a second:
// exactly ticks / 10, as near as a double can hold it. ticks >= 0.
double elapsed_seconds(int ticks);

// The largest number in v, skipping NaNs. NaN when there is no number.
double largest(const std::vector<double>& v);

// A price in dollars as cents, rounded to the nearest cent: 0.29 is 29.
std::int64_t to_cents(double dollars);

// The number in a string of decimal digits, such as an id from a JSON
// file. nullopt if the string is empty, has anything but digits in it, or
// the number doesn't fit in 64 bits.
std::optional<std::uint64_t> parse_id(std::string_view digits);
