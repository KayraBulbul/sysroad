#include "png.hpp"

#include <format>

#include "endian.hpp"

std::optional<PngInfo> parse_png(std::string_view bytes) {
    constexpr std::string_view signature("\x89PNG\r\n\x1a\n", 8);
    if (bytes.size() < 29 || bytes.substr(0, 8) != signature || bytes.substr(12, 4) != "IHDR") {
        return std::nullopt;
    }
    auto* p = reinterpret_cast<const unsigned char*>(bytes.data());
    if (load_be32(p + 8) != 13) {
        return std::nullopt;
    }
    PngInfo info;
    info.width = load_be32(p + 16);
    info.height = load_be32(p + 20);
    info.bit_depth = p[24];
    info.color_type = p[25];
    info.interlaced = p[28] == 1;
    return info;
}

static const char* colour_name(int type) {
    switch (type) {
    case 0:
        return "grey";
    case 2:
        return "RGB";
    case 3:
        return "palette";
    case 4:
        return "grey + alpha";
    case 6:
        return "RGBA";
    default:
        return "unknown";
    }
}

std::string describe(const PngInfo& info) {
    std::string s = std::format("{} x {}, {}-bit {}", info.width, info.height, info.bit_depth,
                                colour_name(info.color_type));
    if (info.interlaced) {
        s += ", interlaced";
    }
    return s;
}
