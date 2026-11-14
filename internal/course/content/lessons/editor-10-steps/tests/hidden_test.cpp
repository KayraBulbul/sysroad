#include "doctest.h"

#include "session.hpp"

namespace ktest {

TEST_CASE("Deletes in a row are one step, growing to the right") {
    Kite k(6, 40);
    k.open("a.txt", "abcdef\n");
    k.press(keys::right).press(keys::del, 2);
    CHECK(k.line(0) == "adef");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "abcdef");
    CHECK(k.cursor() == "row 0, col 1");
    k.press(keys::ctrl_z);
    CHECK(k.line(5) == "Nothing to undo");
}

TEST_CASE("Enter is a step of its own") {
    Kite k(6, 40);
    k.open("ot.txt", "onetwo\n");
    k.press(keys::right, 3).press(keys::enter).type("x");
    CHECK(k.rows(0, 2) == "one\nxtwo\n");
    k.press(keys::ctrl_z);
    CHECK(k.rows(0, 2) == "one\ntwo\n");
    CHECK(k.cursor() == "row 1, col 0");
    k.press(keys::ctrl_z);
    CHECK(k.rows(0, 2) == "onetwo\n~\n");
    CHECK(k.cursor() == "row 0, col 3");
}

TEST_CASE("a Backspace across a line break is a step of its own") {
    Kite k(6, 40);
    k.open("abcd.txt", "ab\ncd\n");
    k.press(keys::down).press(keys::backspace, 2);
    CHECK(k.rows(0, 2) == "acd\n~\n");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "abcd");
    CHECK(k.cursor() == "row 0, col 2");
    k.press(keys::ctrl_z);
    CHECK(k.rows(0, 2) == "ab\ncd\n");
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("typing then erasing are two steps") {
    Kite k(6, 40);
    k.open("e.txt", "\n");
    k.type("ab").press(keys::backspace);
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "ab");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "");
}

TEST_CASE("moving the cursor ends a step, even when typing carries on from the same place") {
    Kite k(6, 40);
    k.open("e.txt", "\n");
    k.type("ab").press(keys::left).press(keys::right).type("cd");
    CHECK(k.line(0) == "abcd");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "ab");
}

TEST_CASE("typing somewhere else is a new step") {
    Kite k(6, 40);
    k.open("ab.txt", "ab\n");
    k.press(keys::end).type("1").press(keys::home).type("2");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "ab1");
}

TEST_CASE("typing after an undo starts a new step") {
    Kite k(6, 40);
    k.open("e.txt", "\n");
    k.type("ab cd").press(keys::ctrl_z);
    CHECK(k.cursor() == "row 0, col 3");
    k.type("x");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "ab");
    CHECK(k.cursor() == "row 0, col 3");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "");
}

TEST_CASE("redo ends a step too") {
    Kite k(6, 40);
    k.open("e.txt", "\n");
    k.type("ab").press(keys::ctrl_z).press(keys::ctrl_y).type("c");
    CHECK(k.line(0) == "abc");
    k.press(keys::ctrl_z);
    CHECK_MESSAGE(k.line(0) == "ab", "the c typed after a redo should undo on its own");
}

TEST_CASE("a search ends a step too") {
    Kite k(6, 40);
    k.open("e.txt", "\n");
    k.type("ab").press(keys::ctrl_f).type("b").press(keys::esc).type("c");
    CHECK(k.line(0) == "abc");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "ab");
}

TEST_CASE("many steps, undone one at a time and redone") {
    Kite k(6, 40);
    k.open("s.txt", "start\n");
    k.press(keys::end).type(" one\ntwo three");
    std::string steps;
    for (int i = 0; i < 5; ++i) {
        k.press(keys::ctrl_z);
        steps += k.line(0) + "|" + k.line(1) + " @" + k.cursor() + "; ";
    }
    CHECK(steps == "start one|two @row 1, col 4; start one| @row 1, col 0; start one|~ @row 0, col 9; "
                   "start|~ @row 0, col 6; start|~ @row 0, col 5; ");
    k.press(keys::ctrl_z);
    CHECK(k.line(5) == "Nothing to undo");
    k.press(keys::ctrl_y, 5);
    CHECK(k.rows(0, 2) == "start one\ntwo three\n");
    CHECK(k.cursor() == "row 1, col 9");
}

}  // namespace ktest
