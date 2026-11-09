#pragma once
// From lesson 2.1.4. You won't need to change this file.

#include <cstdint>

// Read a number stored at p, least significant byte first.
std::uint16_t load_le16(const unsigned char* p);
std::uint32_t load_le32(const unsigned char* p);

// Read a number stored at p, most significant byte first.
std::uint16_t load_be16(const unsigned char* p);
std::uint32_t load_be32(const unsigned char* p);
