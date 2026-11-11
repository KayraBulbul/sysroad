#include "macho.hpp"

#include <format>
#include <stdexcept>

#include "bytes.hpp"

namespace {

constexpr std::uint32_t lc_segment_64 = 0x19;
constexpr std::uint32_t lc_symtab = 0x2;
constexpr std::uint32_t lc_load_dylib = 0xc;
constexpr std::uint32_t lc_load_weak_dylib = 0x80000018;

// A name in a fixed-size field, padded with NULs (and not ending in one
// if it fills the field).
std::string fixed_string(std::string_view b, std::size_t at, std::size_t len) {
    if (at > b.size() || b.size() - at < len) {
        throw std::out_of_range("name field past the end");
    }
    std::string_view f = b.substr(at, len);
    return std::string(f.substr(0, f.find('\0')));
}

// A NUL-terminated string at `at`, which must end before `end`.
std::string c_string(std::string_view b, std::size_t at, std::size_t end) {
    if (at >= end || end > b.size()) {
        throw std::out_of_range("string past the end");
    }
    std::size_t nul = b.find('\0', at);
    if (nul == std::string_view::npos || nul >= end) {
        throw std::out_of_range("string has no end");
    }
    return std::string(b.substr(at, nul - at));
}

}  // namespace

MachHeader parse_mach_header(std::string_view b) {
    std::uint32_t magic = read_u32(b, 0);
    if (magic == 0xbebafeca) {
        throw std::runtime_error("a universal binary: thin it first (lipo -thin arm64)");
    }
    if (magic != 0xfeedfacf) {
        throw std::runtime_error("not a 64-bit little-endian Mach-O file");
    }
    MachHeader h;
    h.cputype = read_u32(b, 4);
    h.cpusubtype = read_u32(b, 8);
    h.filetype = read_u32(b, 12);
    h.ncmds = read_u32(b, 16);
    h.sizeofcmds = read_u32(b, 20);
    h.flags = read_u32(b, 24);
    return h;
}

std::vector<LoadCommand> load_commands(std::string_view b, const MachHeader& h) {
    std::vector<LoadCommand> out;
    std::size_t at = 32;
    for (std::uint32_t i = 0; i < h.ncmds; ++i) {
        LoadCommand c;
        c.cmd = read_u32(b, at);
        c.size = read_u32(b, at + 4);
        c.offset = at;
        if (c.size < 8 || at + c.size > b.size()) {
            throw std::out_of_range("load command " + std::to_string(i) + " has a bad size");
        }
        out.push_back(c);
        at += c.size;
    }
    return out;
}

std::vector<MachSegment> segments(std::string_view b, const std::vector<LoadCommand>& cmds) {
    std::vector<MachSegment> out;
    for (const LoadCommand& c : cmds) {
        if (c.cmd != lc_segment_64) {
            continue;
        }
        std::size_t at = c.offset;
        MachSegment s;
        s.name = fixed_string(b, at + 8, 16);
        s.vmaddr = read_u64(b, at + 24);
        s.vmsize = read_u64(b, at + 32);
        s.fileoff = read_u64(b, at + 40);
        s.filesize = read_u64(b, at + 48);
        std::uint32_t nsects = read_u32(b, at + 64);
        for (std::uint32_t i = 0; i < nsects; ++i) {
            std::size_t sat = at + 72 + 80 * static_cast<std::size_t>(i);
            MachSection sec;
            sec.name = fixed_string(b, sat, 16);
            sec.segment = fixed_string(b, sat + 16, 16);
            sec.addr = read_u64(b, sat + 32);
            sec.size = read_u64(b, sat + 40);
            sec.offset = read_u32(b, sat + 48);
            s.sections.push_back(sec);
        }
        out.push_back(s);
    }
    return out;
}

std::vector<std::string> dylibs(std::string_view b, const std::vector<LoadCommand>& cmds) {
    std::vector<std::string> out;
    for (const LoadCommand& c : cmds) {
        if (c.cmd == lc_load_dylib || c.cmd == lc_load_weak_dylib) {
            // The name's offset is counted from the start of the command.
            std::uint32_t name = read_u32(b, c.offset + 8);
            out.push_back(c_string(b, c.offset + name, c.offset + c.size));
        }
    }
    return out;
}

std::vector<MachSymbol> symbols(std::string_view b, const std::vector<LoadCommand>& cmds) {
    std::vector<MachSymbol> out;
    for (const LoadCommand& c : cmds) {
        if (c.cmd != lc_symtab) {
            continue;
        }
        std::uint32_t symoff = read_u32(b, c.offset + 8);
        std::uint32_t nsyms = read_u32(b, c.offset + 12);
        std::uint32_t stroff = read_u32(b, c.offset + 16);
        std::uint32_t strsize = read_u32(b, c.offset + 20);
        for (std::uint32_t i = 0; i < nsyms; ++i) {
            std::size_t at = symoff + 16 * static_cast<std::size_t>(i);
            MachSymbol s;
            std::uint32_t strx = read_u32(b, at);
            s.name = c_string(b, std::size_t{stroff} + strx, std::size_t{stroff} + strsize);
            s.type = static_cast<std::uint8_t>(read_le(b, at + 4, 1));
            s.sect = static_cast<std::uint8_t>(read_le(b, at + 5, 1));
            s.value = read_u64(b, at + 8);
            out.push_back(s);
        }
        break;
    }
    return out;
}

std::string command_name(std::uint32_t cmd) {
    switch (cmd) {
    case 0x2:
        return "LC_SYMTAB";
    case 0xb:
        return "LC_DYSYMTAB";
    case 0xc:
        return "LC_LOAD_DYLIB";
    case 0xe:
        return "LC_LOAD_DYLINKER";
    case 0x19:
        return "LC_SEGMENT_64";
    case 0x1b:
        return "LC_UUID";
    case 0x1d:
        return "LC_CODE_SIGNATURE";
    case 0x26:
        return "LC_FUNCTION_STARTS";
    case 0x29:
        return "LC_DATA_IN_CODE";
    case 0x2a:
        return "LC_SOURCE_VERSION";
    case 0x2e:
        return "LC_LINKER_OPTIMIZATION_HINT";
    case 0x32:
        return "LC_BUILD_VERSION";
    case 0x80000018:
        return "LC_LOAD_WEAK_DYLIB";
    case 0x80000022:
        return "LC_DYLD_INFO_ONLY";
    case 0x80000028:
        return "LC_MAIN";
    case 0x80000033:
        return "LC_DYLD_EXPORTS_TRIE";
    case 0x80000034:
        return "LC_DYLD_CHAINED_FIXUPS";
    }
    return std::format("{:#x}", cmd);
}
