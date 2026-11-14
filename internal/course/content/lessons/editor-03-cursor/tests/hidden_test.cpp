#include "doctest.h"

#include <unistd.h>

#include "kite_pty.hpp"
#include "session.hpp"

namespace ktest {

TEST_CASE("Left at the start of a line goes to the end of the line above") {
    Kite k(10, 40);
    k.open("t.txt", "abc\nde\n");
    k.press(keys::down).press(keys::left);
    CHECK(k.cursor() == "row 0, col 3");
}

TEST_CASE("Right at the end of a line goes to the start of the next") {
    Kite k(10, 40);
    k.open("t.txt", "abc\nde\n");
    k.press(keys::end).press(keys::right);
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("Down stops on the line just past the end, where the cursor is at 0") {
    Kite k(10, 40);
    k.open("t.txt", "abc\nde\n");
    k.press(keys::right).press(keys::down, 5);
    CHECK(k.cursor() == "row 2, col 0");
    k.press(keys::right);  // nothing to the right of nothing
    CHECK(k.cursor() == "row 2, col 0");
    k.press(keys::end);
    CHECK(k.cursor() == "row 2, col 0");
    CHECK(k.line(8) == "t.txt - 2 lines                      3/2");
}

TEST_CASE("Right at the end of the last line goes to the line past the end") {
    Kite k(10, 40);
    k.open("t.txt", "ab\n");
    k.press(keys::right, 3);
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("moving onto a shorter line snaps to its end, and doesn't remember the column") {
    Kite k(10, 40);
    k.open("t.txt", "a long line\nshort\nanother long line\n");
    k.press(keys::end).press(keys::down);
    CHECK(k.cursor() == "row 1, col 5");
    k.press(keys::down);
    CHECK(k.cursor() == "row 2, col 5");
}

TEST_CASE("Page Down and Page Up move as many lines as the text has rows") {
    Kite k(6, 40);  // 4 rows of text
    k.open("ten.txt", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");
    k.press(keys::page_down);
    CHECK(k.line(4) == "ten.txt - 10 lines                  5/10");
    k.press(keys::page_down, 2);
    CHECK_MESSAGE(k.line(4) == "ten.txt - 10 lines                 11/10", "it stops on the line past the end");
    k.press(keys::page_up);
    CHECK(k.line(4) == "ten.txt - 10 lines                  7/10");
    k.press(keys::page_up, 2);
    CHECK(k.line(4) == "ten.txt - 10 lines                  1/10");
}

TEST_CASE("keys that do nothing don't move it, and an empty file has one place to be") {
    Kite k(10, 40);
    open_file(k.e, "empty.txt");
    for (std::string_view key : {keys::down, keys::right, keys::end, keys::page_down, keys::f5, keys::ctrl_c, keys::esc}) {
        k.press(key);
        CHECK(k.cursor() == "row 0, col 0");
        CHECK_FALSE(k.quit);
    }
}

TEST_CASE("an arrow that arrives in two pieces still moves the cursor") {
    write_file("hello.txt", "hello\n");
    KitePty kite(6, 20, {"hello.txt"});
    REQUIRE_MESSAGE(kite.wait_until([](const Term& t) { return t.line(0) == "hello"; }), kite.shown());
    kite.type("\x1b");
    usleep(20'000);
    kite.type("[C");
    bool moved = kite.wait_until([](const Term& t) { return t.cursor_col() == 1; });
    CHECK_MESSAGE(moved, (std::string("the cursor should be at column 1, after one Right. It's at ") + kite.term().cursor()));
    kite.settle();
    CHECK(kite.term().line(0) == "hello");
    CHECK(kite.term().cursor() == "row 0, col 1");
    kite.type("\x11");
    CHECK(kite.wait_exit() == 0);
}

}  // namespace ktest
