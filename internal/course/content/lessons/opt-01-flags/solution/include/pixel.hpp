#pragma once
// You won't need to change this file.

#include <cstdint>

// p made brighter by amount (or darker, if it's negative), kept within 0-255.
std::uint8_t brighten(std::uint8_t p, int amount);
