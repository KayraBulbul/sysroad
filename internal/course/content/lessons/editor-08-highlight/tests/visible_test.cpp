#include "doctest.h"

#include "codes.hpp"

TEST_CASE("keywords, types, numbers and strings") {
    CHECK(hl("int x = 42;") == "tttnnnnnddn");
    CHECK(hl("return \"hi\";") == "kkkkkknssssn");
}

TEST_CASE("a // comment runs to the end of the line") {
    CHECK(hl("x; // note") == "nnnccccccc");
}

TEST_CASE("separators") {
    CHECK(is_separator(' '));
    CHECK(is_separator(';'));
    CHECK(is_separator('('));
    CHECK_FALSE(is_separator('a'));
    CHECK_FALSE(is_separator('_'));
}
