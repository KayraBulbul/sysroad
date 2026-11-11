#include "sections.hpp"

#include <format>
#include <stdexcept>

#include "bytes.hpp"

std::string string_at(std::string_view b, const Section& s, std::size_t at) {
    if (at >= s.size || s.offset > b.size() || b.size() - s.offset < s.size) {
        throw std::out_of_range("string at " + std::to_string(at) + " is outside " + s.name);
    }
    std::string_view table = b.substr(s.offset, s.size);
    std::size_t end = table.find('\0', at);
    if (end == std::string_view::npos) {
        throw std::out_of_range("string at " + std::to_string(at) + " has no end");
    }
    return std::string(table.substr(at, end - at));
}

std::vector<Section> parse_sections(std::string_view b, const ElfHeader& h) {
    std::vector<Section> out;
    std::vector<std::uint32_t> name_offsets;
    for (std::size_t i = 0; i < h.shnum; ++i) {
        std::size_t at = h.shoff + i * h.shentsize;
        Section s;
        name_offsets.push_back(read_u32(b, at));
        s.type = read_u32(b, at + 4);
        s.flags = read_u64(b, at + 8);
        s.addr = read_u64(b, at + 16);
        s.offset = read_u64(b, at + 24);
        s.size = read_u64(b, at + 32);
        s.link = read_u32(b, at + 40);
        s.info = read_u32(b, at + 44);
        s.align = read_u64(b, at + 48);
        s.entsize = read_u64(b, at + 56);
        out.push_back(s);
    }
    // The names live in a section, so they can only be read once every
    // section header has been.
    if (h.shstrndx >= out.size()) {
        if (out.empty()) {
            return out;
        }
        throw std::out_of_range("e_shstrndx is past the last section");
    }
    const Section names = out[h.shstrndx];
    for (std::size_t i = 0; i < out.size(); ++i) {
        out[i].name = string_at(b, names, name_offsets[i]);
    }
    return out;
}

std::vector<Symbol> parse_symbols(std::string_view b, const std::vector<Section>& sections) {
    std::vector<Symbol> out;
    for (const Section& s : sections) {
        if (s.type != 2) {
            continue;
        }
        if (s.link >= sections.size()) {
            throw std::out_of_range("symbol table's string table is past the last section");
        }
        const Section& strtab = sections[s.link];
        std::size_t entsize = s.entsize != 0 ? s.entsize : 24;
        for (std::size_t at = s.offset; at + entsize <= s.offset + s.size; at += entsize) {
            Symbol sym;
            sym.name = string_at(b, strtab, read_u32(b, at));
            auto info = static_cast<std::uint8_t>(read_le(b, at + 4, 1));
            sym.type = info & 0xf;
            sym.bind = info >> 4;
            sym.shndx = read_u16(b, at + 6);
            sym.value = read_u64(b, at + 8);
            sym.size = read_u64(b, at + 16);
            out.push_back(sym);
        }
        break;
    }
    return out;
}

std::string section_type(std::uint32_t type) {
    switch (type) {
    case 0:
        return "NULL";
    case 1:
        return "PROGBITS";
    case 2:
        return "SYMTAB";
    case 3:
        return "STRTAB";
    case 4:
        return "RELA";
    case 5:
        return "HASH";
    case 6:
        return "DYNAMIC";
    case 7:
        return "NOTE";
    case 8:
        return "NOBITS";
    case 9:
        return "REL";
    case 11:
        return "DYNSYM";
    case 14:
        return "INIT_ARRAY";
    case 15:
        return "FINI_ARRAY";
    case 17:
        return "GROUP";
    case 19:
        return "RELR";
    case 0x6ffffff6:
        return "GNU_HASH";
    case 0x6ffffffe:
        return "VERNEED";
    case 0x6fffffff:
        return "VERSYM";
    }
    return std::format("{:#x}", type);
}

std::string section_flags(std::uint64_t flags) {
    std::string s;
    if (flags & 0x1) {
        s += 'W';
    }
    if (flags & 0x2) {
        s += 'A';
    }
    if (flags & 0x4) {
        s += 'X';
    }
    if (flags & 0x10) {
        s += 'M';
    }
    if (flags & 0x20) {
        s += 'S';
    }
    if (flags & 0x40) {
        s += 'I';
    }
    return s;
}

std::string symbol_type(std::uint8_t type) {
    switch (type) {
    case 0:
        return "NOTYPE";
    case 1:
        return "OBJECT";
    case 2:
        return "FUNC";
    case 3:
        return "SECTION";
    case 4:
        return "FILE";
    case 6:
        return "TLS";
    }
    return std::format("<unknown>: {}", type);
}

std::string symbol_bind(std::uint8_t bind) {
    switch (bind) {
    case 0:
        return "LOCAL";
    case 1:
        return "GLOBAL";
    case 2:
        return "WEAK";
    }
    return std::format("<unknown>: {}", bind);
}

std::string section_index(std::uint16_t shndx) {
    switch (shndx) {
    case 0:
        return "UND";
    case 0xfff1:
        return "ABS";
    case 0xfff2:
        return "COM";
    }
    return std::to_string(shndx);
}
