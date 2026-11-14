#include "doctest.h"

#include "session.hpp"

namespace ktest {

TEST_CASE("a Windows file (\\r\\n) shows the same as a Unix one") {
    Kite k(8, 20);
    k.open("dos.txt", "one\r\ntwo\r\n");
    CHECK(k.rows(0, 3) == "one\ntwo\n~\n");
    k.press(keys::end);
    CHECK_MESSAGE(k.cursor() == "row 0, col 3", "End should stop after the e: the \\r isn't part of the line");
}

TEST_CASE("a tab runs to the next multiple of 8 columns, and the cursor jumps it") {
    Kite k(8, 20);
    k.open("tab.txt", "\tx\n");
    CHECK(k.line(0) == "        x");
    k.press(keys::right);
    CHECK(k.cursor() == "row 0, col 8");
    k.press(keys::right);
    CHECK(k.cursor() == "row 0, col 9");
}

TEST_CASE("moving down past the bottom row scrolls, keeping the cursor on the bottom row") {
    Kite k(5, 20);  // 3 rows of text
    k.open("ten.txt", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");
    k.press(keys::down, 5);
    CHECK(k.rows(0, 3) == "3\n4\n5\n");
    CHECK(k.cursor() == "row 2, col 0");
}

}  // namespace ktest
