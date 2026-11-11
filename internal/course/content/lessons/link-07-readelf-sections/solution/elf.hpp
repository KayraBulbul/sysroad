#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// The ELF header of a 64-bit little-endian file (Elf64_Ehdr).
struct ElfHeader {
    std::uint8_t os_abi = 0;      // e_ident[7]
    std::uint16_t type = 0;       // e_type: 1 relocatable, 2 executable, 3 shared, 4 core
    std::uint16_t machine = 0;    // e_machine: 62 is x86-64, 183 AArch64
    std::uint64_t entry = 0;      // e_entry: where execution starts, or 0
    std::uint64_t phoff = 0;      // e_phoff: file offset of the program header table
    std::uint64_t shoff = 0;      // e_shoff: file offset of the section header table
    std::uint32_t flags = 0;      // e_flags
    std::uint16_t ehsize = 0;     // e_ehsize: this header's size, 64
    std::uint16_t phentsize = 0;  // e_phentsize: one program header's size, 56
    std::uint16_t phnum = 0;      // e_phnum: how many program headers
    std::uint16_t shentsize = 0;  // e_shentsize: one section header's size, 64
    std::uint16_t shnum = 0;      // e_shnum: how many section headers
    std::uint16_t shstrndx = 0;   // e_shstrndx: which section holds the section names
};

// One program header (Elf64_Phdr): a segment.
struct Segment {
    std::uint32_t type = 0;    // p_type: 1 LOAD, 2 DYNAMIC, 3 INTERP, …
    std::uint32_t flags = 0;   // p_flags: 4 read, 2 write, 1 execute
    std::uint64_t offset = 0;  // p_offset: where its bytes start in the file
    std::uint64_t vaddr = 0;   // p_vaddr: where they go in memory
    std::uint64_t filesz = 0;  // p_filesz: how many bytes come from the file
    std::uint64_t memsz = 0;   // p_memsz: how much memory it takes (the rest is zeroed)
    std::uint64_t align = 0;   // p_align
};

// Reads the header. Throws std::runtime_error if b isn't a 64-bit
// little-endian ELF file, and std::out_of_range if it's cut short.
ElfHeader parse_header(std::string_view b);

// Reads the h.phnum program headers. Throws like parse_header if the
// table runs past the end of b.
std::vector<Segment> parse_segments(std::string_view b, const ElfHeader& h);

// As readelf prints them:
//   type_name(2)        "EXEC (Executable file)"
//   machine_name(62)    "Advanced Micro Devices X86-64"
//   segment_type(1)     "LOAD"
//   segment_flags(5)    "R E"
// See the lesson for the full lists, and for what unknown values give.
std::string type_name(std::uint16_t type);
std::string machine_name(std::uint16_t machine);
std::string segment_type(std::uint32_t type);
std::string segment_flags(std::uint32_t flags);
