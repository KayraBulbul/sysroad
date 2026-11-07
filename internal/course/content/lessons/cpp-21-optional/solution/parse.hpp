#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <optional>
#include <string_view>

// The whole of s as an int, or nullopt.
std::optional<int> parse_int(std::string_view s);

// "123", "4K", "2m", "1G" (powers of 1024) in bytes, or nullopt.
std::optional<std::uint64_t> parse_size(std::string_view s);
