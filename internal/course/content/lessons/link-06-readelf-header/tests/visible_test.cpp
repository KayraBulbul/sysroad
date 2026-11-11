#include "doctest.h"

#include "elf.hpp"
#include "samples.hpp"

TEST_CASE("the header of the sample executable") {
    ElfHeader h = parse_header(bytes_of(sample_x86_64_exe));
    CHECK(h.type == 2);
    CHECK(h.machine == 62);
    CHECK(h.entry == 0x201160);
    CHECK(h.phoff == 64);
    CHECK(h.phnum == 5);
    CHECK(h.shnum == 8);
    CHECK(h.shstrndx == 6);
}

TEST_CASE("names") {
    CHECK(type_name(2) == "EXEC (Executable file)");
    CHECK(machine_name(62) == "Advanced Micro Devices X86-64");
    CHECK(segment_type(1) == "LOAD");
    CHECK(segment_flags(5) == "R E");
}

TEST_CASE("the segments of the sample executable") {
    std::string_view b = bytes_of(sample_x86_64_exe);
    std::vector<Segment> segs = parse_segments(b, parse_header(b));
    REQUIRE(segs.size() == 5);
    CHECK(segs[2].type == 1);
    CHECK(segs[2].vaddr == 0x201160);
    CHECK(segs[2].flags == 5);
}
