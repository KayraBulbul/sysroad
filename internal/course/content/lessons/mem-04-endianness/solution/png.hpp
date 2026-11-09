#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

struct PngInfo {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    int bit_depth = 0;
    int color_type = 0;  // 0 grey, 2 RGB, 3 palette, 4 grey + alpha, 6 RGBA
    bool interlaced = false;
};

// The header of a PNG file, or nullopt if bytes don't start with the PNG
// signature followed by a 13-byte IHDR chunk.
std::optional<PngInfo> parse_png(std::string_view bytes);

// "64 x 48, 8-bit RGBA", with ", interlaced" on the end if it is.
std::string describe(const PngInfo& info);
