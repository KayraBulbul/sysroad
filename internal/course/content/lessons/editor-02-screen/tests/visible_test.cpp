#include "doctest.h"

#include <string>

#include "editor.hpp"

static Editor screen(int rows, int cols) {
    Editor e;
    e.screen_rows = rows;
    e.screen_cols = cols;
    return e;
}

TEST_CASE("an empty file: tildes, with the welcome a third of the way down") {
    Editor e = screen(3, 40);
    CHECK(draw_screen(e) == "\x1b[?25l\x1b[H"
                            "~\x1b[K\r\n"
                            "~      kite -- a tiny text editor\x1b[K\r\n"
                            "~\x1b[K"
                            "\x1b[1;1H\x1b[?25h");
}

TEST_CASE("text rows, then tildes, and the cursor where it is") {
    Editor e = screen(3, 20);
    e.rows = {{"hello", "hello"}, {"world", "world"}};
    e.cx = 2;
    e.cy = 1;
    CHECK(draw_screen(e) == "\x1b[?25l\x1b[H"
                            "hello\x1b[K\r\n"
                            "world\x1b[K\r\n"
                            "~\x1b[K"
                            "\x1b[2;3H\x1b[?25h");
}
