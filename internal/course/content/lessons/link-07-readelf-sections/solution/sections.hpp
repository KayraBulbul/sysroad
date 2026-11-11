#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "elf.hpp"

// One section header (Elf64_Shdr), with its name looked up.
struct Section {
    std::string name;
    std::uint32_t type = 0;     // sh_type: 1 PROGBITS, 2 SYMTAB, 3 STRTAB, 8 NOBITS, …
    std::uint64_t flags = 0;    // sh_flags: 1 write, 2 alloc, 4 execute, …
    std::uint64_t addr = 0;     // sh_addr: its address in memory, or 0 in a .o
    std::uint64_t offset = 0;   // sh_offset: where its bytes are in the file
    std::uint64_t size = 0;     // sh_size
    std::uint32_t link = 0;     // sh_link: for a symbol table, its string table
    std::uint32_t info = 0;     // sh_info
    std::uint64_t align = 0;    // sh_addralign
    std::uint64_t entsize = 0;  // sh_entsize: size of one entry, for tables
};

// One symbol (Elf64_Sym), with its name looked up.
struct Symbol {
    std::string name;
    std::uint64_t value = 0;  // st_value: its address, or offset in its section in a .o
    std::uint64_t size = 0;   // st_size
    std::uint8_t type = 0;    // the low 4 bits of st_info: 0 NOTYPE, 1 OBJECT, 2 FUNC, …
    std::uint8_t bind = 0;    // the high 4 bits of st_info: 0 LOCAL, 1 GLOBAL, 2 WEAK
    std::uint16_t shndx = 0;  // st_shndx: the section it's in, or 0 if undefined
};

// The NUL-terminated string at offset `at` in the bytes of section s.
std::string string_at(std::string_view b, const Section& s, std::size_t at);

// All h.shnum section headers, named from section h.shstrndx.
std::vector<Section> parse_sections(std::string_view b, const ElfHeader& h);

// The symbols in the section of type SYMTAB (2), named from the string
// table its `link` points at. Empty if there's no SYMTAB section.
std::vector<Symbol> parse_symbols(std::string_view b, const std::vector<Section>& sections);

// As readelf prints them. See the lesson for the lists.
std::string section_type(std::uint32_t type);    // section_type(1) is "PROGBITS"
std::string section_flags(std::uint64_t flags);  // section_flags(6) is "AX"
std::string symbol_type(std::uint8_t type);      // symbol_type(2) is "FUNC"
std::string symbol_bind(std::uint8_t bind);      // symbol_bind(1) is "GLOBAL"
std::string section_index(std::uint16_t shndx);  // section_index(0) is "UND"
