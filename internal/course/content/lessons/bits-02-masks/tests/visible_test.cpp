#include "doctest.h"

#include "fields.hpp"

TEST_CASE("get_field reads a run of bits") {
    CHECK(get_field(0xabcd, 4, 8) == 0xbc);
    CHECK(get_field(0xabcd, 0, 4) == 0xd);
    CHECK(get_field(0b1011'0000, 4, 1) == 1);
}

TEST_CASE("set_field replaces only those bits") {
    CHECK(set_field(0xabcd, 4, 8, 0x12) == 0xa12d);
    CHECK(set_field(0xffffffff, 8, 4, 0) == 0xfffff0ff);
    CHECK(set_field(0, 28, 4, 0xf) == 0xf0000000);
}

TEST_CASE("unpack565") {
    CHECK(unpack565(0xfd20) == Rgb{255, 166, 0});
    CHECK(unpack565(0xffff) == Rgb{255, 255, 255});
    CHECK(unpack565(0x0000) == Rgb{0, 0, 0});
}

TEST_CASE("pack565") {
    CHECK(pack565(Rgb{255, 166, 0}) == 0xfd20);
    CHECK(pack565(Rgb{255, 255, 255}) == 0xffff);
}

TEST_CASE("mode_string") {
    CHECK(mode_string(0754) == "rwxr-xr--");
    CHECK(mode_string(0644) == "rw-r--r--");
}
