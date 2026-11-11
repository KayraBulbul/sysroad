#include "doctest.h"

#include "macho.hpp"
#include "samples.hpp"

TEST_CASE("the header") {
    MachHeader h = parse_mach_header(sample_macho_exe());
    CHECK(h.cputype == 0x0100000c);
    CHECK(h.filetype == 2);
    CHECK(h.ncmds == 16);
}

TEST_CASE("load commands") {
    std::string b = sample_macho_exe();
    std::vector<LoadCommand> cmds = load_commands(b, parse_mach_header(b));
    REQUIRE(cmds.size() == 16);
    CHECK(cmds[0].cmd == 0x19);
    CHECK(cmds[0].offset == 32);
    CHECK(cmds[1].offset == 32 + cmds[0].size);
    CHECK(command_name(cmds[11].cmd) == "LC_MAIN");
}

TEST_CASE("segments, libraries and symbols") {
    std::string b = sample_macho_exe();
    std::vector<LoadCommand> cmds = load_commands(b, parse_mach_header(b));
    std::vector<MachSegment> segs = segments(b, cmds);
    REQUIRE(segs.size() == 5);
    CHECK(segs[1].name == "__TEXT");
    REQUIRE(segs[1].sections.size() == 5);
    CHECK(segs[1].sections[0].name == "__text");
    CHECK(dylibs(b, cmds) == std::vector<std::string>{"/usr/lib/libSystem.B.dylib"});
    std::vector<MachSymbol> syms = symbols(b, cmds);
    REQUIRE(syms.size() == 5);
    CHECK(syms[1].name == "_main");
}
