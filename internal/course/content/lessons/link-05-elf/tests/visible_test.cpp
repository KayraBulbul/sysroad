#include "doctest.h"

#include <string>

#include "ident.hpp"
#include "samples.hpp"

TEST_CASE("read_u16, read_u32 and read_u64 are little-endian") {
    std::string b = "\x01\x02\x03\x04\x05\x06\x07\x08";
    CHECK(read_u16(b, 0) == 0x0201);
    CHECK(read_u32(b, 0) == 0x04030201);
    CHECK(read_u64(b, 0) == 0x0807060504030201);
    CHECK(read_u16(b, 6) == 0x0807);
}

TEST_CASE("identify the samples") {
    CHECK(identify(bytes_of(sample_x86_64_o)) == "ELF 64-bit little-endian");
    CHECK(identify(bytes_of(sample_macho_o)) == "Mach-O 64-bit little-endian");
}

TEST_CASE("identify other kinds") {
    CHECK(identify("!<arch>\n") == "ar archive");
    CHECK(identify("#!/bin/sh\necho hi\n") == "script");
    CHECK(identify("hello") == "unknown");
}
