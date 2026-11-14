#include "doctest.h"

#include "colours.hpp"

namespace ktest {

TEST_CASE("in a C++ file, numbers are red and strings magenta") {
    CHECK(hl("x = 42;") == "nnnnddn");
    CHECK(hl("s = \"hi\";") == "nnnnssssn");
}

TEST_CASE("a file that isn't C or C++ has no colours") {
    CHECK(hl("x = 42; s = \"hi\";", "notes.txt") == "nnnnnnnnnnnnnnnnn");
}

TEST_CASE("control characters show inverted, as letters") {
    Kite k(6, 40);
    k.open("ctrl.txt", "a\x01" "b\x1b" "c\n");
    CHECK(k.line(0) == "aAb[c");
    CHECK(colours(k, 0, 5) == "n#n#n");
}

TEST_CASE("the search's match is inverted while the search is on") {
    Kite k(6, 40);
    k.open("cats.txt", "the cat\na cat sat\n");
    k.press(keys::ctrl_f).type("cat");
    CHECK(colours(k, 0, 7) == "nnnn###");
    k.press(keys::enter);
    CHECK(colours(k, 0, 7) == "nnnnnnn");
}

}  // namespace ktest
