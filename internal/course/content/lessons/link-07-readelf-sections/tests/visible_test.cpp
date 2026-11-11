#include "doctest.h"

#include "samples.hpp"
#include "sections.hpp"

static std::vector<Section> sections_of(std::string_view b) {
    return parse_sections(b, parse_header(b));
}

TEST_CASE("section names and types") {
    std::vector<Section> s = sections_of(bytes_of(sample_x86_64_o));
    REQUIRE(s.size() == 9);
    CHECK(s[0].name == "");
    CHECK(s[2].name == ".text");
    CHECK(s[2].type == 1);
    CHECK(s[2].size == 0x2a);
    CHECK(s[6].name == ".bss");
    CHECK(s[8].name == ".symtab");
}

TEST_CASE("symbols") {
    std::string_view b = bytes_of(sample_x86_64_o);
    std::vector<Symbol> syms = parse_symbols(b, sections_of(b));
    REQUIRE(syms.size() == 11);
    CHECK(syms[2].name == "hidden");
    CHECK(syms[4].name == "add");
    CHECK(syms[4].type == 2);
    CHECK(syms[4].bind == 1);
    CHECK(syms[4].size == 10);
    CHECK(syms[10].name == "puts");
    CHECK(syms[10].shndx == 0);
}

TEST_CASE("names as readelf prints them") {
    CHECK(section_type(8) == "NOBITS");
    CHECK(section_flags(6) == "AX");
    CHECK(symbol_type(2) == "FUNC");
    CHECK(symbol_bind(1) == "GLOBAL");
    CHECK(section_index(0) == "UND");
    CHECK(section_index(4) == "4");
}
