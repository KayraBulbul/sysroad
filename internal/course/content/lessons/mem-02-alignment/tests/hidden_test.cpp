#include "doctest.h"

#include <cstddef>
#include <cstdint>

#include "layout.hpp"

struct A {
    char a;
    std::int16_t b;
    char c;
    double d;
    char e;
};

struct B {
    char tag[3];
    char x;
};

struct C {
    double d;
    int i;
};

TEST_CASE("a mixed struct matches the compiler") {
    Layout l = lay_out({{"a", 1, 1}, {"b", 2, 2}, {"c", 1, 1}, {"d", 8, 8}, {"e", 1, 1}});
    REQUIRE(l.fields.size() == 5);
    CHECK(l.fields[1].offset == offsetof(A, b));
    CHECK(l.fields[1].padding_before == 1);
    CHECK(l.fields[2].offset == offsetof(A, c));
    CHECK(l.fields[3].offset == offsetof(A, d));
    CHECK(l.fields[3].padding_before == 3);
    CHECK(l.fields[4].offset == offsetof(A, e));
    CHECK(l.size == sizeof(A));
    CHECK(l.align == alignof(A));
    CHECK(l.tail_padding == 7);
}

TEST_CASE("chars only: no padding") {
    Layout l = lay_out({{"tag", 3, 1}, {"x", 1, 1}});
    CHECK(l.size == sizeof(B));
    CHECK(l.align == 1);
    CHECK(l.tail_padding == 0);
    CHECK(l.fields[1].padding_before == 0);
}

TEST_CASE("the size is rounded up to the alignment") {
    Layout l = lay_out({{"d", 8, 8}, {"i", 4, 4}});
    CHECK(l.size == sizeof(C));
    CHECK(l.tail_padding == 4);
}

TEST_CASE("a single field") {
    Layout l = lay_out({{"x", 4, 4}});
    CHECK(l.size == 4);
    CHECK(l.align == 4);
    CHECK(l.fields[0].offset == 0);
}
