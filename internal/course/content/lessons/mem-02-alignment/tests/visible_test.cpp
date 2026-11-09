#include "doctest.h"

#include <cstddef>

#include "layout.hpp"

TEST_CASE("align_up") {
    CHECK(align_up(0, 8) == 0);
    CHECK(align_up(5, 4) == 8);
    CHECK(align_up(8, 4) == 8);
    CHECK(align_up(9, 1) == 9);
    CHECK(align_up(17, 16) == 32);
}

struct S {
    char c;
    int n;
    char d;
};

TEST_CASE("char, int, char matches the compiler") {
    Layout l = lay_out({{"c", 1, 1}, {"n", 4, 4}, {"d", 1, 1}});
    REQUIRE(l.fields.size() == 3);
    CHECK(l.fields[0].offset == offsetof(S, c));
    CHECK(l.fields[1].offset == offsetof(S, n));
    CHECK(l.fields[1].padding_before == 3);
    CHECK(l.fields[2].offset == offsetof(S, d));
    CHECK(l.size == sizeof(S));
    CHECK(l.align == alignof(S));
    CHECK(l.tail_padding == 3);
}
