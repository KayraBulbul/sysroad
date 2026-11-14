#include "doctest.h"

#include "session.hpp"

namespace ktest {

TEST_CASE("typing into an empty file adds its first line") {
    Kite k(8, 30);
    open_file(k.e, "new.txt");
    k.type("hi");
    CHECK(k.rows(0, 2) == "hi\n~\n");
    CHECK(k.cursor() == "row 0, col 2");
}

TEST_CASE("typing on the line past the end adds that line") {
    Kite k(8, 30);
    k.open("one.txt", "one\n");
    k.press(keys::down).type("two");
    CHECK(k.rows(0, 3) == "one\ntwo\n~\n");
    CHECK(k.cursor() == "row 1, col 3");
}

TEST_CASE("Enter at a line's start, at its end, and on the line past the end") {
    Kite k(8, 30);
    k.open("mid.txt", "mid\n");
    k.press(keys::enter);
    CHECK(k.rows(0, 3) == "\nmid\n~\n");
    CHECK(k.cursor() == "row 1, col 0");
    k.press(keys::end).press(keys::enter);
    CHECK(k.rows(0, 4) == "\nmid\n\n~\n");
    CHECK(k.cursor() == "row 2, col 0");
    k.press(keys::down).press(keys::enter);
    CHECK(k.rows(0, 5) == "\nmid\n\n\n~\n");
    CHECK(k.cursor() == "row 4, col 0");
}

TEST_CASE("Backspace at the start of a line joins it onto the line above") {
    Kite k(8, 30);
    k.open("hw.txt", "hello\nworld\n");
    k.press(keys::down).press(keys::backspace);
    CHECK(k.rows(0, 2) == "helloworld\n~\n");
    CHECK(k.cursor() == "row 0, col 5");
}

TEST_CASE("Backspace at the very start, or on the line past the end, does nothing") {
    Kite k(8, 30);
    k.open("ab.txt", "ab\n");
    k.press(keys::backspace);
    CHECK(k.rows(0, 2) == "ab\n~\n");
    CHECK(k.cursor() == "row 0, col 0");
    k.press(keys::down).press(keys::backspace);
    CHECK(k.rows(0, 2) == "ab\n~\n");
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("Ctrl-H is Backspace too") {
    Kite k(8, 30);
    k.open("ab.txt", "ab\n");
    k.press(keys::end).press("\x08");
    CHECK(k.line(0) == "a");
}

TEST_CASE("a tab is one character, drawn to the next tab stop") {
    Kite k(8, 30);
    k.open("x.txt", "x\n");
    k.press(keys::tab);
    CHECK(k.line(0) == "        x");
    CHECK(k.cursor() == "row 0, col 8");
    k.press(keys::backspace);
    CHECK(k.line(0) == "x");
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("Delete removes the character at the cursor, joins at a line's end, and stops at the text's end") {
    Kite k(8, 30);
    k.open("abcd.txt", "ab\ncd\n");
    k.press(keys::del);
    CHECK(k.rows(0, 3) == "b\ncd\n~\n");
    k.press(keys::end).press(keys::del);
    CHECK(k.rows(0, 2) == "bcd\n~\n");
    k.press(keys::end).press(keys::del);
    CHECK(k.rows(0, 2) == "bcd\n~\n");
    CHECK(k.cursor() == "row 0, col 3");
    k.press(keys::down).press(keys::del);
    CHECK(k.rows(0, 2) == "bcd\n~\n");
}

TEST_CASE("control keys and sequences kite doesn't use aren't typed") {
    Kite k(8, 30);
    k.open("x.txt", "x\n");
    k.press(keys::ctrl_c).press(keys::f5).press(keys::esc).press("\x01").press("\x1bOP");
    CHECK(k.rows(0, 2) == "x\n~\n");
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("bytes above 127 are typed as they are") {
    Kite k(8, 30);
    k.open("cafe.txt", "caf\n");
    k.press(keys::end).type("\xc3\xa9");
    CHECK(k.line(0) == "caf\xc3\xa9");
}

TEST_CASE("splitting a line and joining it back") {
    Kite k(8, 30);
    k.open("abcdef.txt", "abcdef\n");
    k.press(keys::right, 3).press(keys::enter).press(keys::backspace);
    CHECK(k.rows(0, 2) == "abcdef\n~\n");
    CHECK(k.cursor() == "row 0, col 3");
}

TEST_CASE("typing past the right edge scrolls, and Enter at the bottom scrolls down") {
    Kite k(4, 10);  // 2 rows of text
    k.open("ab.txt", "a\nb\n");
    k.press(keys::down).press(keys::end).type("0123456789xyz");
    CHECK(k.line(1) == "456789xyz");
    CHECK(k.cursor() == "row 1, col 9");
    k.press(keys::enter);  // the view scrolls down a line, and back to column 0
    CHECK(k.line(0) == "b012345678");
    CHECK(k.line(1) == "");
    CHECK(k.cursor() == "row 1, col 0");
}

}  // namespace ktest
