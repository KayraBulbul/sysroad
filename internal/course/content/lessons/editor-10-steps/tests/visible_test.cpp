#include "doctest.h"

#include "session.hpp"

namespace ktest {

TEST_CASE("a typed word undoes in one step, and redoes in one") {
    Kite k(6, 40);
    k.open("e.txt", "\n");
    k.type("hello").press(keys::ctrl_z);
    CHECK(k.line(0) == "");
    CHECK(k.cursor() == "row 0, col 0");
    k.press(keys::ctrl_y);
    CHECK(k.line(0) == "hello");
    CHECK(k.cursor() == "row 0, col 5");
}

TEST_CASE("each word is a step: a word after a space starts a new one") {
    Kite k(6, 40);
    k.open("e.txt", "\n");
    k.type("hello world").press(keys::ctrl_z);
    CHECK(k.line(0) == "hello");  // and a space: the cursor is after it
    CHECK(k.cursor() == "row 0, col 6");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "");
}

TEST_CASE("Backspaces in a row are one step, and undo puts the cursor back where they started") {
    Kite k(6, 40);
    k.open("a.txt", "abcdef\n");
    k.press(keys::right, 5).press(keys::backspace, 3);
    CHECK(k.line(0) == "abf");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "abcdef");
    CHECK(k.cursor() == "row 0, col 5");
    k.press(keys::ctrl_y);
    CHECK(k.line(0) == "abf");
    CHECK(k.cursor() == "row 0, col 2");
}

}  // namespace ktest
