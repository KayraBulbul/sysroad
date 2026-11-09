#include "doctest.h"

#include "keys.hpp"
#include "text.hpp"

TEST_CASE("typing into an empty file adds its first line") {
    Editor e;
    type(e, "hi");
    CHECK(text(e) == "hi");
    CHECK(e.cy == 0);
    CHECK(e.cx == 2);
}

TEST_CASE("typing on the line past the end adds that line") {
    Editor e = with_lines({"one"});
    e.cy = 1;
    type(e, "two");
    CHECK(text(e) == "one|two");
}

TEST_CASE("Enter at the start, the end, and past the end") {
    Editor e = with_lines({"mid"});
    insert_newline(e);
    CHECK(text(e) == "|mid");
    CHECK(e.cy == 1);
    e.cx = 3;
    insert_newline(e);
    CHECK(text(e) == "|mid|");
    CHECK(e.cy == 2);
    e.cy = 3;
    e.cx = 0;
    insert_newline(e);
    CHECK(text(e) == "|mid||");
    CHECK(e.cy == 4);
}

TEST_CASE("Backspace at the start of a line joins it to the one above") {
    Editor e = with_lines({"hello", "world"});
    e.cy = 1;
    delete_char(e);
    CHECK(text(e) == "helloworld");
    CHECK(e.cy == 0);
    CHECK(e.cx == 5);
}

TEST_CASE("Backspace at the very start, or past the end, does nothing") {
    Editor e = with_lines({"ab"});
    delete_char(e);
    CHECK(text(e) == "ab");
    e.cy = 1;
    delete_char(e);
    CHECK(text(e) == "ab");
    CHECK(e.cy == 1);
}

TEST_CASE("tabs are characters; render keeps up") {
    Editor e = with_lines({"x"});
    insert_char(e, '\t');
    CHECK(e.rows[0].chars == "\tx");
    CHECK(e.rows[0].render == "        x");
    e.cx = 1;
    delete_char(e);
    CHECK(e.rows[0].render == "x");
}

TEST_CASE("Delete, through the keys: forward, joining, and nothing at the end") {
    Editor e = with_lines({"ab", "cd"});
    process_key(e, kDelete);
    CHECK(text(e) == "b|cd");
    process_key(e, kEnd);
    process_key(e, kDelete);
    CHECK(text(e) == "bcd");
    process_key(e, kEnd);
    process_key(e, kDelete);
    CHECK(text(e) == "bcd");
    CHECK(e.cy == 0);
    CHECK(e.cx == 3);
}

TEST_CASE("a round trip: split, then join back") {
    Editor e = with_lines({"abcdef"});
    e.cx = 3;
    insert_newline(e);
    delete_char(e);
    CHECK(text(e) == "abcdef");
    CHECK(e.cx == 3);
    CHECK(e.cy == 0);
}
