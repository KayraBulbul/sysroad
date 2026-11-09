#include "bmp.hpp"

#include <format>

#include "reader.hpp"

BmpInfo parse_bmp(std::span<const unsigned char> file) {
    ByteReader r(file);
    auto magic = r.bytes(2);
    if (!r.ok() || magic[0] != 'B' || magic[1] != 'M') {
        throw BmpError("not a BMP file");
    }
    r.skip(8);  // file size, reserved
    std::uint32_t offset = r.le32();
    std::uint32_t dib = r.le32();
    auto width = static_cast<std::int32_t>(r.le32());
    auto height = static_cast<std::int32_t>(r.le32());
    r.skip(2);  // planes
    int bits = r.le16();
    std::uint32_t compression = r.le32();
    r.skip(20);  // the rest of the 40-byte info header
    if (!r.ok()) {
        throw BmpError("truncated header");
    }
    if (dib < 40) {
        throw BmpError(std::format("unsupported header size {}", dib));
    }
    if (width <= 0 || height == 0 || height == INT32_MIN) {
        throw BmpError(std::format("bad size {} x {}", width, height));
    }
    if (bits != 24 && bits != 32) {
        throw BmpError(std::format("unsupported: {} bits per pixel", bits));
    }
    if (compression != 0 && !(compression == 3 && bits == 32)) {
        throw BmpError(std::format("unsupported compression {}", compression));
    }

    BmpInfo info;
    info.width = width;
    info.top_down = height < 0;
    info.height = info.top_down ? -height : height;
    info.bits = bits;
    info.pixel_offset = offset;
    std::uint64_t row = (std::uint64_t(bits) * width + 31) / 32 * 4;
    if (offset + row * info.height > file.size()) {
        throw BmpError("pixel data is truncated");
    }
    info.row_size = static_cast<std::uint32_t>(row);
    return info;
}

Rgb pixel_at(std::span<const unsigned char> file, const BmpInfo& info, int x, int y) {
    if (x < 0 || y < 0 || x >= info.width || y >= info.height) {
        throw BmpError(
            std::format("({}, {}) is outside the {} x {} image", x, y, info.width, info.height));
    }
    std::size_t row = info.top_down ? y : info.height - 1 - y;
    std::size_t at = info.pixel_offset + row * info.row_size + std::size_t(x) * (info.bits / 8);
    return {file[at + 2], file[at + 1], file[at]};
}
