#include "doctest.h"

#include "session.hpp"

namespace ktest {

TEST_CASE("Ctrl-Z takes back the last edit, and Ctrl-Y does it again") {
    Kite k(6, 40);
    k.open("hello.txt", "hello\n");
    k.press(keys::end).type("!");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "hello");
    CHECK(k.cursor() == "row 0, col 5");
    k.press(keys::ctrl_y);
    CHECK(k.line(0) == "hello!");
    CHECK(k.cursor() == "row 0, col 6");
}

TEST_CASE("with nothing to undo or redo, kite says so") {
    Kite k(6, 40);
    k.open("x.txt", "x\n");
    k.press(keys::ctrl_z);
    CHECK(k.line(5) == "Nothing to undo");
    k.press(keys::ctrl_y);
    CHECK(k.line(5) == "Nothing to redo");
    CHECK(k.line(0) == "x");
}

TEST_CASE("undo all the way back, then redo all the way") {
    Kite k(6, 40);
    k.open("start.txt", "start\n");
    k.press(keys::end).type(" ab\ncd");
    std::string done = k.screen();
    k.press(keys::ctrl_z, 12);
    CHECK(k.rows(0, 2) == "start\n~\n");
    CHECK(k.cursor() == "row 0, col 5");
    CHECK(k.line(5) == "Nothing to undo");
    k.press(keys::ctrl_y, 12);
    CHECK(k.rows(0, 3) == "start ab\ncd\n~\n");
    CHECK(k.cursor() == "row 1, col 2");
}

}  // namespace ktest
