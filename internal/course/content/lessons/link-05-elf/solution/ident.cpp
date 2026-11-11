#include "ident.hpp"

#include <stdexcept>

namespace {

// n bytes from at, least significant first.
std::uint64_t read_le(std::string_view b, std::size_t at, std::size_t n) {
    if (at > b.size() || b.size() - at < n) {
        throw std::out_of_range("reading " + std::to_string(n) + " bytes at " + std::to_string(at) +
                                " of " + std::to_string(b.size()));
    }
    std::uint64_t v = 0;
    for (std::size_t i = 0; i < n; ++i) {
        v |= std::uint64_t{static_cast<unsigned char>(b[at + i])} << (8 * i);
    }
    return v;
}

std::uint32_t read_be32(std::string_view b, std::size_t at) {
    std::uint32_t le = static_cast<std::uint32_t>(read_le(b, at, 4));
    return (le >> 24) | ((le >> 8) & 0xff00) | ((le << 8) & 0xff0000) | (le << 24);
}

}  // namespace

std::uint16_t read_u16(std::string_view b, std::size_t at) {
    return static_cast<std::uint16_t>(read_le(b, at, 2));
}

std::uint32_t read_u32(std::string_view b, std::size_t at) {
    return static_cast<std::uint32_t>(read_le(b, at, 4));
}

std::uint64_t read_u64(std::string_view b, std::size_t at) {
    return read_le(b, at, 8);
}

std::string identify(std::string_view b) {
    if (b.starts_with("\177ELF") && b.size() >= 6) {  // \177 is 0x7f, in octal
        const char* bits = b[4] == 1 ? "32-bit" : b[4] == 2 ? "64-bit" : nullptr;
        const char* order = b[5] == 1 ? "little-endian" : b[5] == 2 ? "big-endian" : nullptr;
        if (bits == nullptr || order == nullptr) {
            return "unknown";
        }
        return std::string("ELF ") + bits + " " + order;
    }
    if (b.size() >= 4) {
        switch (read_u32(b, 0)) {
        case 0xfeedfacf:
            return "Mach-O 64-bit little-endian";
        case 0xfeedface:
            return "Mach-O 32-bit little-endian";
        case 0xcffaedfe:
            return "Mach-O 64-bit big-endian";
        case 0xcefaedfe:
            return "Mach-O 32-bit big-endian";
        case 0xbebafeca:
            // Fat Mach-O and Java class files share 0xcafebabe. A fat binary
            // next has a small count of CPU types; a class file, its version.
            if (b.size() >= 8 && read_be32(b, 4) >= 1 && read_be32(b, 4) < 20) {
                return "Mach-O universal";
            }
            return "Java class";
        }
    }
    if (b.starts_with("!<arch>\n")) {
        return "ar archive";
    }
    if (b.starts_with("#!")) {
        return "script";
    }
    if (b.starts_with("MZ")) {
        return "DOS/Windows executable";
    }
    return "unknown";
}
