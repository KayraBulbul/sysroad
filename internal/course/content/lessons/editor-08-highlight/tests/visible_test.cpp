#include "doctest.h"

#include "colours.hpp"

namespace ktest {

TEST_CASE("keywords are yellow and types green, among the numbers and strings") {
    CHECK(hl("int x = 42;") == "tttnnnnnddn");
    CHECK(hl("return \"hi\";") == "kkkkkknssssn");
}

TEST_CASE("a // comment is cyan to the end of the line") {
    CHECK(hl("x; // note") == "nnnccccccc");
}

TEST_CASE("a /* comment */ runs across lines") {
    Kite k(8, 40);
    k.open("block.cpp", "int /* open\nstill inside\nend */ 1\n");
    CHECK(colours(k, 0, 11) == "tttnccccccc");
    CHECK(colours(k, 1, 12) == "cccccccccccc");
    CHECK(colours(k, 2, 8) == "ccccccnd");
}

}  // namespace ktest
