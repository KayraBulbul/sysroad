#include "doctest.h"

#include "session.hpp"

namespace ktest {

TEST_CASE("the arrows move the cursor a step") {
    Kite k(10, 40);
    k.open("hw.txt", "hello\nworld\n");
    k.press(keys::right, 3);
    CHECK(k.cursor() == "row 0, col 3");
    k.press(keys::down);
    CHECK(k.cursor() == "row 1, col 3");
    k.press(keys::left);
    CHECK(k.cursor() == "row 1, col 2");
    k.press(keys::up);
    CHECK(k.cursor() == "row 0, col 2");
}

TEST_CASE("the cursor stops at the top left") {
    Kite k(10, 40);
    k.open("hello.txt", "hello\n");
    k.press(keys::up).press(keys::left);
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("Home and End go to the ends of the line") {
    Kite k(10, 40);
    k.open("hello.txt", "hello\n");
    k.press(keys::end);
    CHECK(k.cursor() == "row 0, col 5");
    k.press(keys::home);
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("Ctrl-Q says it's time to quit, and other keys don't") {
    Kite k(10, 40);
    k.open("hello.txt", "hello\n");
    k.press(keys::right);
    CHECK_FALSE(k.quit);
    k.press(keys::ctrl_q);
    CHECK_MESSAGE(k.quit, "process_key should return false for Ctrl-Q");
}

TEST_CASE("the status bar follows the cursor's line") {
    Kite k(10, 40);
    k.open("five.txt", "1\n2\n3\n4\n5\n");
    k.press(keys::down, 2);
    CHECK(k.line(8) == "five.txt - 5 lines                   3/5");
}

}  // namespace ktest
