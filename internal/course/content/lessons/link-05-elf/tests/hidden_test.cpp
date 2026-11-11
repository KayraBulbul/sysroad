#include "doctest.h"

#include <stdexcept>
#include <string>

#include "ident.hpp"
#include "samples.hpp"

using namespace std::string_literals;

TEST_CASE("bytes above 127 aren't sign-extended") {
    std::string b = "\xff\xfe\x80\x90\xa0\xb0\xc0\xd0"s;
    CHECK(read_u16(b, 0) == 0xfeff);
    CHECK(read_u32(b, 0) == 0x9080feff);
    CHECK(read_u64(b, 0) == 0xd0c0b0a09080feffULL);
}

TEST_CASE("reading past the end throws") {
    std::string b = "\x01\x02\x03\x04"s;
    CHECK(read_u32(b, 0) == 0x04030201);
    CHECK_THROWS_AS(read_u32(b, 1), std::out_of_range);
    CHECK_THROWS_AS(read_u16(b, 3), std::out_of_range);
    CHECK_THROWS_AS(read_u64(b, 0), std::out_of_range);
    CHECK_THROWS_AS(read_u16(b, 1000), std::out_of_range);
    CHECK_THROWS_AS(read_u16(b, static_cast<std::size_t>(-1)), std::out_of_range);
    CHECK(read_u16(b, 2) == 0x0403);
}

TEST_CASE("the ELF samples") {
    CHECK(identify(bytes_of(sample_arm64_o)) == "ELF 64-bit little-endian");
    CHECK(identify(bytes_of(sample_x86_64_exe)) == "ELF 64-bit little-endian");
    // e_type, just after the 16 identification bytes: 1 is a .o, 2 an executable.
    CHECK(read_u16(bytes_of(sample_x86_64_o), 16) == 1);
    CHECK(read_u16(bytes_of(sample_x86_64_exe), 16) == 2);
}

TEST_CASE("32-bit and big-endian ELF") {
    CHECK(identify("\x7f"
                   "ELF\x01\x01\x01"s) == "ELF 32-bit little-endian");
    CHECK(identify("\x7f"
                   "ELF\x02\x02\x01"s) == "ELF 64-bit big-endian");
    CHECK(identify("\x7f"
                   "ELF\x01\x02\x01"s) == "ELF 32-bit big-endian");
    CHECK(identify("\x7f"
                   "ELF\x03\x01\x01"s) == "unknown");
    CHECK(identify("\x7f"
                   "ELF\x02\x00\x01"s) == "unknown");
    CHECK(identify("\x7f"
                   "EL"s) == "unknown");
}

TEST_CASE("every Mach-O magic") {
    CHECK(identify("\xcf\xfa\xed\xfe\x0c\x00\x00\x01"s) == "Mach-O 64-bit little-endian");
    CHECK(identify("\xce\xfa\xed\xfe\x07\x00\x00\x00"s) == "Mach-O 32-bit little-endian");
    CHECK(identify("\xfe\xed\xfa\xcf\x00\x00\x00\x12"s) == "Mach-O 64-bit big-endian");
    CHECK(identify("\xfe\xed\xfa\xce\x00\x00\x00\x12"s) == "Mach-O 32-bit big-endian");
}

TEST_CASE("cafebabe: universal binary or Java class") {
    CHECK(identify("\xca\xfe\xba\xbe\x00\x00\x00\x02"s) == "Mach-O universal");
    CHECK(identify("\xca\xfe\xba\xbe\x00\x00\x00\x41"s) == "Java class");  // Java 21 is version 65
}

TEST_CASE("short and empty input") {
    CHECK(identify("") == "unknown");
    CHECK(identify("#") == "unknown");
    CHECK(identify("#!") == "script");
    CHECK(identify("MZ\x90\x00"s) == "DOS/Windows executable");
    CHECK(identify("!<arch>") == "unknown");
}
