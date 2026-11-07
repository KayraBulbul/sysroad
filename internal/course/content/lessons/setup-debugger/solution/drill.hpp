#pragma once
// The drill's functions. You won't need to change this file.

#include <cstdint>
#include <string>
#include <string_view>

std::uint32_t step(std::uint32_t x);
std::uint32_t walk(std::uint32_t seed, int n);
std::uint64_t checksum(int n);
std::string make_name(std::uint32_t seed);
int level_of(std::string_view name);
