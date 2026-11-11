#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// mach_header_64, without the magic and the reserved field.
struct MachHeader {
    std::uint32_t cputype = 0;  // 0x0100000c arm64, 0x01000007 x86-64
    std::uint32_t cpusubtype = 0;
    std::uint32_t filetype = 0;    // 1 object, 2 executable, 6 dylib, 8 bundle
    std::uint32_t ncmds = 0;       // how many load commands follow the header
    std::uint32_t sizeofcmds = 0;  // their total size in bytes
    std::uint32_t flags = 0;
};

// Where a load command is, and what kind it is.
struct LoadCommand {
    std::uint32_t cmd = 0;   // LC_SEGMENT_64 is 0x19, …
    std::uint32_t size = 0;  // cmdsize, including these 8 bytes
    std::size_t offset = 0;  // where it starts in the file
};

struct MachSection {
    std::string segment;  // segname, e.g. "__TEXT"
    std::string name;     // sectname, e.g. "__text"
    std::uint64_t addr = 0;
    std::uint64_t size = 0;
    std::uint32_t offset = 0;  // where its bytes are in the file, 0 for zero-fill
};

// An LC_SEGMENT_64 and the sections in it.
struct MachSegment {
    std::string name;  // segname; empty in an object file
    std::uint64_t vmaddr = 0;
    std::uint64_t vmsize = 0;
    std::uint64_t fileoff = 0;
    std::uint64_t filesize = 0;
    std::vector<MachSection> sections;
};

// One nlist_64 entry of the LC_SYMTAB symbol table.
struct MachSymbol {
    std::string name;       // with Mach-O's leading underscore: "_main"
    std::uint8_t type = 0;  // n_type: 0x01 external, (n_type & 0x0e) 0 undefined, 0x0e in a section
    std::uint8_t sect = 0;  // n_sect: 1-based section number, 0 for none
    std::uint64_t value = 0;
};

// Reads a 64-bit little-endian Mach-O header. Throws std::runtime_error for
// anything else (a universal binary should be thinned first), and
// std::out_of_range if b is cut short.
MachHeader parse_mach_header(std::string_view b);

// The h.ncmds load commands that follow the 32-byte header.
std::vector<LoadCommand> load_commands(std::string_view b, const MachHeader& h);

// Every LC_SEGMENT_64, with its sections.
std::vector<MachSegment> segments(std::string_view b, const std::vector<LoadCommand>& cmds);

// The install names of the libraries the file links against: one per
// LC_LOAD_DYLIB (0xc) or LC_LOAD_WEAK_DYLIB (0x80000018), in order.
std::vector<std::string> dylibs(std::string_view b, const std::vector<LoadCommand>& cmds);

// The symbols in the LC_SYMTAB table. Empty if there's no LC_SYMTAB.
std::vector<MachSymbol> symbols(std::string_view b, const std::vector<LoadCommand>& cmds);

// "LC_SEGMENT_64" and the rest, as otool prints them. See the lesson.
std::string command_name(std::uint32_t cmd);
