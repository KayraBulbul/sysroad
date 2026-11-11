#include "doctest.h"

#include <stdexcept>
#include <string>

#include "macho.hpp"
#include "samples.hpp"

using namespace std::string_literals;

static std::vector<LoadCommand> cmds_of(std::string_view b) {
    return load_commands(b, parse_mach_header(b));
}

TEST_CASE("every header field") {
    MachHeader h = parse_mach_header(sample_macho_exe());
    CHECK(h.cpusubtype == 0);
    CHECK(h.sizeofcmds == 1368);
    CHECK(h.flags == 0x200085);
    MachHeader o = parse_mach_header(bytes_of(sample_macho_o));
    CHECK(o.filetype == 1);
    CHECK(o.ncmds == 5);
}

TEST_CASE("every segment of the executable") {
    std::string b = sample_macho_exe();
    std::vector<MachSegment> segs = segments(b, cmds_of(b));
    REQUIRE(segs.size() == 5);
    CHECK(segs[0].name == "__PAGEZERO");
    CHECK(segs[0].vmaddr == 0);
    CHECK(segs[0].vmsize == 0x100000000);
    CHECK(segs[0].sections.empty());
    CHECK(segs[1].vmaddr == 0x100000000);
    CHECK(segs[1].fileoff == 0);
    CHECK(segs[1].filesize == 0x4000);
    CHECK(segs[1].sections[0].segment == "__TEXT");
    CHECK(segs[1].sections[0].addr == 0x100000598);
    CHECK(segs[1].sections[0].size == 0x20);
    CHECK(segs[1].sections[0].offset == 0x598);
    CHECK(segs[1].sections[4].name == "__unwind_info");
    CHECK(segs[2].name == "__DATA_CONST");
    CHECK(segs[3].sections[1].name == "__data");
    CHECK(segs[4].name == "__LINKEDIT");
}

TEST_CASE("the object file: one unnamed segment") {
    std::string_view b = bytes_of(sample_macho_o);
    std::vector<LoadCommand> cmds = cmds_of(b);
    std::vector<MachSegment> segs = segments(b, cmds);
    REQUIRE(segs.size() == 1);
    CHECK(segs[0].name == "");
    REQUIRE(segs[0].sections.size() == 5);
    CHECK(segs[0].sections[3].name == "__bss");
    CHECK(segs[0].sections[3].offset == 0);
    CHECK(segs[0].sections[4].segment == "__LD");
    CHECK(segs[0].sections[4].name == "__compact_unwind");  // 16 characters: no NUL at the end
    CHECK(dylibs(b, cmds).empty());
}

TEST_CASE("every symbol of the object file") {
    std::string_view b = bytes_of(sample_macho_o);
    std::vector<MachSymbol> syms = symbols(b, cmds_of(b));
    REQUIRE(syms.size() == 13);
    CHECK(syms[1].name == "_hidden");
    CHECK(syms[1].type == 0x0e);
    CHECK(syms[1].sect == 4);
    CHECK(syms[1].value == 0xd0);
    CHECK(syms[6].name == "_add");
    CHECK(syms[6].type == 0x0f);
    CHECK(syms[6].sect == 1);
    CHECK(syms[12].name == "_puts");
    CHECK(syms[12].type == 0x01);
    CHECK(syms[12].sect == 0);
}

TEST_CASE("executable symbols") {
    std::string b = sample_macho_exe();
    std::vector<MachSymbol> syms = symbols(b, cmds_of(b));
    REQUIRE(syms.size() == 5);
    CHECK(syms[1].value == 0x100000598);
    CHECK(syms[3].name == "_puts");
    CHECK(syms[4].name == "dyld_stub_binder");
}

TEST_CASE("all the command names") {
    CHECK(command_name(0x2) == "LC_SYMTAB");
    CHECK(command_name(0xb) == "LC_DYSYMTAB");
    CHECK(command_name(0xc) == "LC_LOAD_DYLIB");
    CHECK(command_name(0xe) == "LC_LOAD_DYLINKER");
    CHECK(command_name(0x19) == "LC_SEGMENT_64");
    CHECK(command_name(0x1b) == "LC_UUID");
    CHECK(command_name(0x1d) == "LC_CODE_SIGNATURE");
    CHECK(command_name(0x26) == "LC_FUNCTION_STARTS");
    CHECK(command_name(0x29) == "LC_DATA_IN_CODE");
    CHECK(command_name(0x2a) == "LC_SOURCE_VERSION");
    CHECK(command_name(0x2e) == "LC_LINKER_OPTIMIZATION_HINT");
    CHECK(command_name(0x32) == "LC_BUILD_VERSION");
    CHECK(command_name(0x80000018) == "LC_LOAD_WEAK_DYLIB");
    CHECK(command_name(0x80000022) == "LC_DYLD_INFO_ONLY");
    CHECK(command_name(0x80000028) == "LC_MAIN");
    CHECK(command_name(0x80000033) == "LC_DYLD_EXPORTS_TRIE");
    CHECK(command_name(0x80000034) == "LC_DYLD_CHAINED_FIXUPS");
    CHECK(command_name(0x1234) == "0x1234");
}

TEST_CASE("files it can't read") {
    CHECK_THROWS_AS(parse_mach_header(bytes_of(sample_x86_64_o)), std::runtime_error);
    CHECK_THROWS_AS(parse_mach_header("\xca\xfe\xba\xbe\x00\x00\x00\x02"s), std::runtime_error);
    CHECK_THROWS_AS(parse_mach_header("\xcf\xfa\xed\xfe"s), std::out_of_range);
    std::string b = sample_macho_exe();
    MachHeader h = parse_mach_header(b);
    CHECK_THROWS_AS(load_commands(b.substr(0, 600), h), std::out_of_range);
    std::string zero_size = b;
    zero_size[36] = zero_size[37] = zero_size[38] = zero_size[39] = 0;  // the first cmdsize
    CHECK_THROWS_AS(load_commands(zero_size, h), std::out_of_range);
}
