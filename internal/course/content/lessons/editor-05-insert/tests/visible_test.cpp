#include "doctest.h"

#include "session.hpp"

namespace ktest {

TEST_CASE("typing inserts at the cursor and moves it on") {
    Kite k(8, 30);
    k.open("hed.txt", "hed\n");
    k.press(keys::right, 2).type("llo worl");
    CHECK(k.line(0) == "hello world");
    CHECK(k.cursor() == "row 0, col 10");
}

TEST_CASE("Enter splits the line at the cursor") {
    Kite k(8, 30);
    k.open("hw.txt", "helloworld\n");
    k.press(keys::right, 5).press(keys::enter);
    CHECK(k.rows(0, 3) == "hello\nworld\n~\n");
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("Backspace deletes the character before the cursor") {
    Kite k(8, 30);
    k.open("abc.txt", "abc\n");
    k.press(keys::right, 2).press(keys::backspace);
    CHECK(k.line(0) == "ac");
    CHECK(k.cursor() == "row 0, col 1");
}

}  // namespace ktest
