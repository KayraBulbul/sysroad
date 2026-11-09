#include "png.hpp"

#include <format>

#include "endian.hpp"

std::optional<PngInfo> parse_png(std::string_view bytes) {
    // TODO: check the size, signature, chunk length and type, then read the fields.
    (void)bytes;
    return std::nullopt;
}

std::string describe(const PngInfo& info) {
    // TODO
    (void)info;
    return "";
}
