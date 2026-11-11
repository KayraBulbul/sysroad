#include "doctest.h"

#include <stdexcept>
#include <string>

#include "elf.hpp"
#include "samples.hpp"

using namespace std::string_literals;

TEST_CASE("every header field of the sample executable") {
    ElfHeader h = parse_header(bytes_of(sample_x86_64_exe));
    CHECK(h.os_abi == 0);
    CHECK(h.shoff == 608);
    CHECK(h.flags == 0);
    CHECK(h.ehsize == 64);
    CHECK(h.phentsize == 56);
    CHECK(h.shentsize == 64);
}

TEST_CASE("object files") {
    ElfHeader x = parse_header(bytes_of(sample_x86_64_o));
    CHECK(x.type == 1);
    CHECK(x.entry == 0);
    CHECK(x.phnum == 0);
    CHECK(x.phoff == 0);
    CHECK(x.shnum == 9);
    CHECK(x.shstrndx == 1);
    CHECK(parse_segments(bytes_of(sample_x86_64_o), x).empty());
    ElfHeader a = parse_header(bytes_of(sample_arm64_o));
    CHECK(a.machine == 183);
    CHECK(a.type == 1);
    CHECK(a.shnum == 9);
}

TEST_CASE("every segment of the sample executable") {
    std::string_view b = bytes_of(sample_x86_64_exe);
    std::vector<Segment> s = parse_segments(b, parse_header(b));
    REQUIRE(s.size() == 5);
    CHECK(s[0].type == 6);  // PHDR
    CHECK(s[0].offset == 64);
    CHECK(s[0].filesz == 5 * 56);
    CHECK(s[1].type == 1);
    CHECK(s[1].offset == 0);
    CHECK(s[1].vaddr == 0x200000);
    CHECK(s[1].filesz == 0x15e);
    CHECK(s[1].flags == 4);
    CHECK(s[1].align == 0x1000);
    CHECK(s[3].vaddr == 0x202174);
    CHECK(s[3].memsz == 4);
    CHECK(s[3].flags == 6);
    CHECK(s[4].type == 0x6474e551);  // GNU_STACK
}

TEST_CASE("all the names") {
    CHECK(type_name(1) == "REL (Relocatable file)");
    CHECK(type_name(3) == "DYN (Shared object file)");
    CHECK(type_name(4) == "CORE (Core file)");
    CHECK(type_name(0) == "NONE (None)");
    CHECK(type_name(0x1234) == "<unknown>: 0x1234");
    CHECK(machine_name(3) == "Intel 80386");
    CHECK(machine_name(40) == "ARM");
    CHECK(machine_name(183) == "AArch64");
    CHECK(machine_name(243) == "RISC-V");
    CHECK(machine_name(9999) == "<unknown>: 0x270f");
    CHECK(segment_type(0) == "NULL");
    CHECK(segment_type(2) == "DYNAMIC");
    CHECK(segment_type(3) == "INTERP");
    CHECK(segment_type(4) == "NOTE");
    CHECK(segment_type(6) == "PHDR");
    CHECK(segment_type(7) == "TLS");
    CHECK(segment_type(0x6474e550) == "GNU_EH_FRAME");
    CHECK(segment_type(0x6474e551) == "GNU_STACK");
    CHECK(segment_type(0x6474e552) == "GNU_RELRO");
    CHECK(segment_type(0x6474e553) == "GNU_PROPERTY");
    CHECK(segment_type(0x70000001) == "0x70000001");
    CHECK(segment_flags(0) == "   ");
    CHECK(segment_flags(4) == "R  ");
    CHECK(segment_flags(6) == "RW ");
    CHECK(segment_flags(7) == "RWE");
    CHECK(segment_flags(1) == "  E");
}

TEST_CASE("files it can't read") {
    CHECK_THROWS_AS(parse_header("hello, world, this is long enough"), std::runtime_error);
    CHECK_THROWS_AS(parse_header(""), std::runtime_error);
    std::string elf32(bytes_of(sample_x86_64_o));
    elf32[4] = 1;
    CHECK_THROWS_AS(parse_header(elf32), std::runtime_error);
    std::string big(bytes_of(sample_x86_64_o));
    big[5] = 2;
    CHECK_THROWS_AS(parse_header(big), std::runtime_error);
    std::string cut(bytes_of(sample_x86_64_exe).substr(0, 40));
    CHECK_THROWS_AS(parse_header(cut), std::out_of_range);
    std::string_view b = bytes_of(sample_x86_64_exe);
    ElfHeader h = parse_header(b);
    CHECK_THROWS_AS(parse_segments(b.substr(0, 200), h), std::out_of_range);
}
