// readelf -S -s, for 64-bit little-endian ELF files. You won't need to
// change this file.
//
//   readelf FILE          any ELF file, like /bin/ls on Linux
//   readelf x86-64.o      one of the built-in samples: x86-64.o (the
//                         default), arm64.o or x86-64-exe
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>

#include "elf.hpp"
#include "samples.hpp"
#include "sections.hpp"

static std::string load(const char* name) {
    if (std::strcmp(name, "x86-64-exe") == 0) {
        return std::string(bytes_of(sample_x86_64_exe));
    }
    if (std::strcmp(name, "x86-64.o") == 0) {
        return std::string(bytes_of(sample_x86_64_o));
    }
    if (std::strcmp(name, "arm64.o") == 0) {
        return std::string(bytes_of(sample_arm64_o));
    }
    std::ifstream in(name, std::ios::binary);
    if (!in) {
        throw std::runtime_error(std::string("can't open ") + name);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void print_sections(const std::vector<Section>& sections) {
    std::printf("Section Headers:\n");
    std::printf("  [Nr] %-17s %-15s %-16s %-6s %-6s %2s %3s %2s %3s %2s\n", "Name", "Type",
                "Address", "Off", "Size", "ES", "Flg", "Lk", "Inf", "Al");
    for (std::size_t i = 0; i < sections.size(); ++i) {
        const Section& s = sections[i];
        std::printf("  [%2zu] %-17s %-15s %016llx %06llx %06llx %02llx %3s %2u %3u %2llu\n", i,
                    s.name.c_str(), section_type(s.type).c_str(), (unsigned long long)s.addr,
                    (unsigned long long)s.offset, (unsigned long long)s.size,
                    (unsigned long long)s.entsize, section_flags(s.flags).c_str(), s.link, s.info,
                    (unsigned long long)s.align);
    }
}

static void print_symbols(const std::vector<Symbol>& symbols) {
    if (symbols.empty()) {
        std::printf("\nNo symbol table.\n");
        return;
    }
    std::printf("\nSymbol table contains %zu entries:\n", symbols.size());
    std::printf("   Num:    Value          Size Type    Bind   Ndx Name\n");
    for (std::size_t i = 0; i < symbols.size(); ++i) {
        const Symbol& s = symbols[i];
        std::printf("  %4zu: %016llx %5llu %-7s %-6s %3s %s\n", i, (unsigned long long)s.value,
                    (unsigned long long)s.size, symbol_type(s.type).c_str(),
                    symbol_bind(s.bind).c_str(), section_index(s.shndx).c_str(), s.name.c_str());
    }
}

int main(int argc, char** argv) {
    try {
        std::string b = load(argc > 1 ? argv[1] : "x86-64.o");
        ElfHeader h = parse_header(b);
        std::vector<Section> sections = parse_sections(b, h);
        print_sections(sections);
        print_symbols(parse_symbols(b, sections));
    } catch (const std::exception& e) {
        std::fprintf(stderr, "readelf: %s\n", e.what());
        return 1;
    }
}
