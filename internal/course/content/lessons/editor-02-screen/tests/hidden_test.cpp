#include "doctest.h"

#include <string>

#include "editor.hpp"

static Editor screen(int rows, int cols) {
    Editor e;
    e.screen_rows = rows;
    e.screen_cols = cols;
    return e;
}

TEST_CASE("long lines are cut at the screen's width") {
    Editor e = screen(1, 4);
    e.rows = {{"abcdefgh", "abcdefgh"}};
    CHECK(draw_screen(e) == "\x1b[?25l\x1b[Habcd\x1b[K\x1b[1;1H\x1b[?25h");
}

TEST_CASE("a narrow screen cuts the welcome too, with no room for a tilde") {
    Editor e = screen(1, 10);
    CHECK(draw_screen(e) == "\x1b[?25l\x1b[Hkite -- a \x1b[K\x1b[1;1H\x1b[?25h");
}

TEST_CASE("the welcome is centred, and an odd gap rounds down") {
    Editor e = screen(6, 29);  // 3 spare columns: 1 on the left
    std::string out = draw_screen(e);
    CHECK(out.find("~\x1b[K\r\n~\x1b[K\r\n~kite -- a tiny text editor\x1b[K\r\n") !=
          std::string::npos);
}

TEST_CASE("the welcome is only for an empty file") {
    Editor e = screen(3, 40);
    e.rows = {{"x", "x"}};
    CHECK(draw_screen(e).find("kite") == std::string::npos);
}

TEST_CASE("rows beyond the screen aren't drawn; the last line has no newline") {
    Editor e = screen(2, 10);
    e.rows = {{"1", "1"}, {"2", "2"}, {"3", "3"}};
    e.cx = 1;
    CHECK(draw_screen(e) == "\x1b[?25l\x1b[H1\x1b[K\r\n2\x1b[K\x1b[1;2H\x1b[?25h");
}

TEST_CASE("it draws render, not chars") {
    Editor e = screen(1, 20);
    e.rows = {{"a\tb", "a       b"}};
    CHECK(draw_screen(e).find("a       b") != std::string::npos);
}
