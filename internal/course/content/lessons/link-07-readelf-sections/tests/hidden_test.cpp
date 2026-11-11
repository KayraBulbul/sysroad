#include "doctest.h"

#include <stdexcept>
#include <string>

#include "samples.hpp"
#include "sections.hpp"

static std::vector<Section> sections_of(std::string_view b) {
    return parse_sections(b, parse_header(b));
}

TEST_CASE("every field of a section header") {
    std::vector<Section> s = sections_of(bytes_of(sample_x86_64_o));
    REQUIRE(s.size() == 9);
    const Section& rela = s[3];
    CHECK(rela.name == ".rela.text");
    CHECK(rela.type == 4);
    CHECK(rela.flags == 0x40);
    CHECK(rela.offset == 0x180);
    CHECK(rela.size == 0x90);
    CHECK(rela.link == 8);
    CHECK(rela.info == 2);
    CHECK(rela.align == 8);
    CHECK(rela.entsize == 0x18);
    CHECK(s[1].name == ".strtab");  // the section names and symbol names share it here
    CHECK(s[8].link == 1);
}

TEST_CASE("an executable's sections have addresses") {
    std::vector<Section> s = sections_of(bytes_of(sample_x86_64_exe));
    REQUIRE(s.size() == 8);
    CHECK(s[2].name == ".text");
    CHECK(s[2].addr == 0x201160);
    CHECK(s[6].name == ".shstrtab");
    CHECK(s[4].name == ".comment");
}

TEST_CASE("every symbol of the executable") {
    std::string_view b = bytes_of(sample_x86_64_exe);
    std::vector<Symbol> syms = parse_symbols(b, sections_of(b));
    REQUIRE(syms.size() == 5);
    CHECK(syms[1].name == "tiny.c");
    CHECK(syms[1].type == 4);
    CHECK(syms[1].shndx == 0xfff1);
    CHECK(syms[2].name == "_start");
    CHECK(syms[2].value == 0x201160);
    CHECK(syms[2].size == 17);
    CHECK(syms[3].name == "counter");
    CHECK(syms[3].value == 0x202174);
    CHECK(syms[3].type == 1);
}

TEST_CASE("the arm64 object") {
    std::string_view b = bytes_of(sample_arm64_o);
    std::vector<Section> s = sections_of(b);
    std::vector<Symbol> syms = parse_symbols(b, s);
    REQUIRE(syms.size() == 15);
    CHECK(syms[8].name == "add");
    CHECK(syms[8].size == 20);
    CHECK(syms[14].name == "puts");
    CHECK(syms[2].bind == 0);
}

TEST_CASE("no symbol table") {
    std::string_view b = bytes_of(sample_x86_64_o);
    std::vector<Section> s = sections_of(b);
    s[8].type = 1;  // pretend .symtab is something else
    CHECK(parse_symbols(b, s).empty());
}

TEST_CASE("string_at") {
    std::string_view b = bytes_of(sample_x86_64_o);
    std::vector<Section> s = sections_of(b);
    const Section& strtab = s[1];
    CHECK(string_at(b, strtab, 0) == "");
    CHECK_THROWS_AS(string_at(b, strtab, strtab.size), std::out_of_range);
    CHECK_THROWS_AS(string_at(b, strtab, 100000), std::out_of_range);
}

TEST_CASE("all the names") {
    CHECK(section_type(0) == "NULL");
    CHECK(section_type(2) == "SYMTAB");
    CHECK(section_type(3) == "STRTAB");
    CHECK(section_type(4) == "RELA");
    CHECK(section_type(6) == "DYNAMIC");
    CHECK(section_type(7) == "NOTE");
    CHECK(section_type(9) == "REL");
    CHECK(section_type(11) == "DYNSYM");
    CHECK(section_type(14) == "INIT_ARRAY");
    CHECK(section_type(15) == "FINI_ARRAY");
    CHECK(section_type(0x6ffffff6) == "GNU_HASH");
    CHECK(section_type(0x6fffffff) == "VERSYM");
    CHECK(section_type(0x6ffffffe) == "VERNEED");
    CHECK(section_type(0x12345) == "0x12345");
    CHECK(section_flags(0) == "");
    CHECK(section_flags(3) == "WA");
    CHECK(section_flags(0x32) == "AMS");
    CHECK(section_flags(0x40) == "I");
    CHECK(symbol_type(0) == "NOTYPE");
    CHECK(symbol_type(1) == "OBJECT");
    CHECK(symbol_type(3) == "SECTION");
    CHECK(symbol_type(4) == "FILE");
    CHECK(symbol_type(6) == "TLS");
    CHECK(symbol_bind(0) == "LOCAL");
    CHECK(symbol_bind(2) == "WEAK");
    CHECK(section_index(0xfff1) == "ABS");
    CHECK(section_index(0xfff2) == "COM");
    CHECK(section_index(65) == "65");
}
