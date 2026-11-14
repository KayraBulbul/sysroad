#include "doctest.h"

#include <string>

#include "session.hpp"

namespace ktest {

TEST_CASE("\\r\\n endings, a missing last newline, and empty lines") {
    Kite k(8, 20);
    k.open("mixed.txt", "a\r\n\r\nb\n\nlast");
    CHECK(k.rows(0, 6) == "a\n\nb\n\nlast\n~\n");
    CHECK(k.line(6) == "mixed.txt - 5 lines");
}

TEST_CASE("tabs after text go to the next stop, and a tab at a stop takes all 8") {
    Kite k(8, 40);
    k.open("tabs.txt", "ab\tc\t\td\n12345678\tx\n");
    CHECK(k.line(0) == "ab      c               d");
    CHECK(k.line(1) == "12345678        x");
    std::string seen;
    for (int i = 0; i < 7; ++i) {
        k.press(keys::right);
        seen += std::to_string(k.term.cursor_col()) + " ";
    }
    CHECK_MESSAGE(seen == "1 2 8 9 16 24 25 ", "the screen columns after each Right");
}

TEST_CASE("moving up past the top row scrolls back") {
    Kite k(5, 20);
    k.open("ten.txt", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");
    k.press(keys::down, 8).press(keys::up, 7);
    CHECK(k.rows(0, 3) == "1\n2\n3\n");
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("the view stays put while the cursor is on screen") {
    Kite k(7, 20);  // 5 rows of text
    k.open("ten.txt", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");
    k.press(keys::down, 4);
    CHECK(k.line(0) == "0");
    CHECK(k.cursor() == "row 4, col 0");
    k.press(keys::down).press(keys::up, 3);
    CHECK(k.line(0) == "1");
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("a long line scrolls sideways, and back") {
    Kite k(5, 10);
    k.open("long.txt", "0123456789abcdef\nshort\n");
    k.press(keys::end);
    CHECK(k.line(0) == "789abcdef");
    CHECK(k.cursor() == "row 0, col 9");
    CHECK(k.line(1) == "");
    k.press(keys::home);
    CHECK(k.line(0) == "0123456789");
    CHECK(k.line(1) == "short");
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("sideways scrolling counts the columns tabs take on screen") {
    Kite k(5, 10);
    k.open("tabs.txt", "\t\tx\n");
    k.press(keys::right, 2);  // on the x, 16 columns in
    CHECK(k.line(0) == "         x");
    CHECK(k.cursor() == "row 0, col 9");
}

TEST_CASE("moving down from a long line to a short one scrolls left, just enough") {
    Kite k(5, 10);
    k.open("long.txt", "0123456789abcdef\nshort\n");
    k.press(keys::end).press(keys::down);  // the cursor snaps to column 5
    CHECK(k.line(0) == "56789abcde");
    CHECK(k.line(1) == "");
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("the line past the end can be scrolled to") {
    Kite k(4, 20);  // 2 rows of text
    k.open("abc.txt", "a\nb\nc\n");
    k.press(keys::down, 3);
    CHECK(k.rows(0, 2) == "c\n~\n");
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("Page Down in a long file keeps the cursor on screen") {
    Kite k(6, 20);  // 4 rows of text
    std::string text;
    for (int i = 0; i < 20; ++i) {
        text += std::to_string(i) + "\n";
    }
    k.open("twenty.txt", text);
    k.press(keys::page_down);
    CHECK(k.rows(0, 4) == "1\n2\n3\n4\n");
    CHECK(k.cursor() == "row 3, col 0");
    k.press(keys::page_down).press(keys::page_up);
    CHECK(k.rows(0, 4) == "4\n5\n6\n7\n");
    CHECK(k.cursor() == "row 0, col 0");
}

}  // namespace ktest
