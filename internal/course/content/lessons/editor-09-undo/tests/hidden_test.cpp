#include "doctest.h"

#include "colours.hpp"

namespace ktest {

TEST_CASE("undoing Enter joins the lines again") {
    Kite k(6, 40);
    k.open("ot.txt", "onetwo\n");
    k.press(keys::right, 3).press(keys::enter);
    CHECK(k.rows(0, 2) == "one\ntwo\n");
    k.press(keys::ctrl_z);
    CHECK(k.rows(0, 2) == "onetwo\n~\n");
    CHECK(k.cursor() == "row 0, col 3");
    k.press(keys::ctrl_y);
    CHECK(k.rows(0, 2) == "one\ntwo\n");
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("undoing Backspace puts the character back, with the cursor after it") {
    Kite k(6, 40);
    k.open("abc.txt", "abc\n");
    k.press(keys::end).press(keys::backspace);
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "abc");
    CHECK(k.cursor() == "row 0, col 3");
    k.press(keys::ctrl_y);
    CHECK(k.line(0) == "ab");
    CHECK(k.cursor() == "row 0, col 2");
}

TEST_CASE("undoing a Backspace that joined two lines splits them again") {
    Kite k(6, 40);
    k.open("abcd.txt", "ab\ncd\n");
    k.press(keys::down).press(keys::backspace);
    CHECK(k.rows(0, 2) == "abcd\n~\n");
    k.press(keys::ctrl_z);
    CHECK(k.rows(0, 3) == "ab\ncd\n~\n");
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("undoing Delete, in a line and at its end") {
    Kite k(6, 40);
    k.open("abcd.txt", "ab\ncd\n");
    k.press(keys::del);
    k.press(keys::ctrl_z);
    CHECK(k.rows(0, 2) == "ab\ncd\n");
    CHECK(k.cursor() == "row 0, col 0");
    k.press(keys::end).press(keys::del);
    CHECK(k.rows(0, 2) == "abcd\n~\n");
    k.press(keys::ctrl_z);
    CHECK(k.rows(0, 3) == "ab\ncd\n~\n");
    CHECK(k.cursor() == "row 0, col 2");
}

TEST_CASE("edits in different places are undone newest first") {
    Kite k(6, 40);
    k.open("abc.txt", "abc\n");
    k.press(keys::end).type("1").press(keys::home).type("2");
    CHECK(k.line(0) == "2abc1");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "abc1");
    CHECK(k.cursor() == "row 0, col 0");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "abc");
    CHECK(k.cursor() == "row 0, col 3");
}

TEST_CASE("a new edit after an undo empties redo") {
    Kite k(6, 40);
    k.open("abc.txt", "abc\n");
    k.press(keys::end).type("x").press(keys::ctrl_z).type("y").press(keys::ctrl_y);
    CHECK(k.line(5) == "Nothing to redo");
    CHECK(k.line(0) == "abcy");
}

TEST_CASE("typing on the line past the end, undone, leaves no extra line") {
    Kite k(6, 40);
    k.open("one.txt", "one\n");
    k.press(keys::down).type("x");
    CHECK(k.rows(0, 3) == "one\nx\n~\n");
    k.press(keys::ctrl_z);
    CHECK(k.rows(0, 2) == "one\n~\n");
    CHECK(k.cursor() == "row 1, col 0");
}

TEST_CASE("undo marks the file modified, even right after a save") {
    Kite k(6, 40);
    k.open("saved.txt", "saved\n");
    k.press(keys::end).type("!").press(keys::ctrl_s).press(keys::ctrl_z);
    CHECK(k.line(4) == "saved.txt - 1 lines (modified)       1/1");
}

TEST_CASE("undo goes back to where the edit was, scrolling to it") {
    std::string text;
    for (int i = 0; i < 20; ++i) {
        text += "line " + std::to_string(i) + "\n";
    }
    Kite k(6, 40);  // 4 rows of text
    k.open("lines.txt", text);
    k.type("x").press(keys::page_down, 3);
    CHECK(k.line(0) != "xline 0");
    k.press(keys::ctrl_z);
    CHECK(k.line(0) == "line 0");
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("undo keeps the colours right") {
    Kite k(6, 40);
    k.open("c.cpp", "x\ny\n");
    k.press(keys::end).type(" /* open");
    CHECK(colours(k, 1, 1) == "c");
    k.press(keys::ctrl_z, 10);
    CHECK(k.line(0) == "x");
    CHECK(colours(k, 0, 1) == "n");
    CHECK(colours(k, 1, 1) == "n");
    k.press(keys::ctrl_y, 10);
    CHECK(colours(k, 1, 1) == "c");
}

TEST_CASE("a mix of edits all over, undone, gives back the file exactly") {
    Kite k(10, 40);
    k.open("mix.txt", "alpha\nbeta\ngamma\n");
    k.press(keys::down).press(keys::end).type(" one\ntwo").press(keys::up, 2).press(keys::del, 3);
    k.press(keys::down, 3).press(keys::backspace, 4).type("\t!").press(keys::home).press(keys::backspace);
    std::string end = k.screen();
    k.press(keys::ctrl_z, 40);
    CHECK(k.rows(0, 4) == "alpha\nbeta\ngamma\n~\n");
    k.press(keys::ctrl_y, 40);
    CHECK(k.rows(0, 8) == end.substr(0, end.find("mix.txt")));
}

}  // namespace ktest
