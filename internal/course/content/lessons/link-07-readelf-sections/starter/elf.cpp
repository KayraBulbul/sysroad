// The course's version of lesson 5.3.6. You won't need to change this file.
#include "elf.hpp"

#include <format>
#include <stdexcept>

#include "bytes.hpp"

ElfHeader parse_header(std::string_view b) {
    if (b.size() < 16 || !b.starts_with("\177ELF")) {
        throw std::runtime_error("not an ELF file");
    }
    if (b[4] != 2 || b[5] != 1) {
        throw std::runtime_error("only 64-bit little-endian ELF is supported");
    }
    ElfHeader h;
    h.os_abi = static_cast<std::uint8_t>(b[7]);
    h.type = read_u16(b, 16);
    h.machine = read_u16(b, 18);
    h.entry = read_u64(b, 24);
    h.phoff = read_u64(b, 32);
    h.shoff = read_u64(b, 40);
    h.flags = read_u32(b, 48);
    h.ehsize = read_u16(b, 52);
    h.phentsize = read_u16(b, 54);
    h.phnum = read_u16(b, 56);
    h.shentsize = read_u16(b, 58);
    h.shnum = read_u16(b, 60);
    h.shstrndx = read_u16(b, 62);
    return h;
}

std::vector<Segment> parse_segments(std::string_view b, const ElfHeader& h) {
    std::vector<Segment> out;
    for (std::size_t i = 0; i < h.phnum; ++i) {
        std::size_t at = h.phoff + i * h.phentsize;
        Segment s;
        s.type = read_u32(b, at);
        s.flags = read_u32(b, at + 4);
        s.offset = read_u64(b, at + 8);
        s.vaddr = read_u64(b, at + 16);
        s.filesz = read_u64(b, at + 32);
        s.memsz = read_u64(b, at + 40);
        s.align = read_u64(b, at + 48);
        out.push_back(s);
    }
    return out;
}

std::string type_name(std::uint16_t type) {
    switch (type) {
    case 0:
        return "NONE (None)";
    case 1:
        return "REL (Relocatable file)";
    case 2:
        return "EXEC (Executable file)";
    case 3:
        return "DYN (Shared object file)";
    case 4:
        return "CORE (Core file)";
    }
    return std::format("<unknown>: {:#x}", type);
}

std::string machine_name(std::uint16_t machine) {
    switch (machine) {
    case 3:
        return "Intel 80386";
    case 40:
        return "ARM";
    case 62:
        return "Advanced Micro Devices X86-64";
    case 183:
        return "AArch64";
    case 243:
        return "RISC-V";
    }
    return std::format("<unknown>: {:#x}", machine);
}

std::string segment_type(std::uint32_t type) {
    switch (type) {
    case 0:
        return "NULL";
    case 1:
        return "LOAD";
    case 2:
        return "DYNAMIC";
    case 3:
        return "INTERP";
    case 4:
        return "NOTE";
    case 6:
        return "PHDR";
    case 7:
        return "TLS";
    case 0x6474e550:
        return "GNU_EH_FRAME";
    case 0x6474e551:
        return "GNU_STACK";
    case 0x6474e552:
        return "GNU_RELRO";
    case 0x6474e553:
        return "GNU_PROPERTY";
    case 0x6474e554:
        return "GNU_SFRAME";
    }
    return std::format("{:#x}", type);
}

std::string segment_flags(std::uint32_t flags) {
    std::string s = "   ";
    if (flags & 4) {
        s[0] = 'R';
    }
    if (flags & 2) {
        s[1] = 'W';
    }
    if (flags & 1) {
        s[2] = 'E';
    }
    return s;
}
