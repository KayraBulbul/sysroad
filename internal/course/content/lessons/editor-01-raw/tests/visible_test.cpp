#include "doctest.h"

#include "keys.hpp"
#include "terminal.hpp"

TEST_CASE("ordinary bytes are keys by themselves") {
    CHECK(decode_key("a").key == 'a');
    CHECK(decode_key("a").used == 1);
    CHECK(decode_key("\r").key == kEnter);
    CHECK(decode_key("\x11").key == ctrl_key('q'));
    CHECK(decode_key("xyz").used == 1);  // one key at a time
}

TEST_CASE("arrow keys are ESC [ A to D") {
    CHECK(decode_key("\x1b[A").key == kArrowUp);
    CHECK(decode_key("\x1b[B").key == kArrowDown);
    CHECK(decode_key("\x1b[C").key == kArrowRight);
    CHECK(decode_key("\x1b[D").key == kArrowLeft);
    CHECK(decode_key("\x1b[D").used == 3);
}

TEST_CASE("raw mode switches off echo and line buffering") {
    termios t{};
    t.c_lflag = ECHO | ICANON;
    termios raw = raw_attrs(t);
    CHECK((raw.c_lflag & ECHO) == 0);
    CHECK((raw.c_lflag & ICANON) == 0);
}
