#include "doctest.h"

#include <string>

#include "keys.hpp"
#include "terminal.hpp"

TEST_CASE("bytes above 127 aren't negative") {
    CHECK(decode_key("\xc3\xa9").key == 0xc3);
    CHECK(decode_key("\xff").key == 255);
}

TEST_CASE("Home, End, Delete, Page Up and Page Down") {
    CHECK(decode_key("\x1b[H").key == kHome);
    CHECK(decode_key("\x1b[F").key == kEnd);
    CHECK(decode_key("\x1bOH").key == kHome);
    CHECK(decode_key("\x1bOF").key == kEnd);
    CHECK(decode_key("\x1b[1~").key == kHome);
    CHECK(decode_key("\x1b[7~").key == kHome);
    CHECK(decode_key("\x1b[4~").key == kEnd);
    CHECK(decode_key("\x1b[8~").key == kEnd);
    CHECK(decode_key("\x1b[3~").key == kDelete);
    CHECK(decode_key("\x1b[5~").key == kPageUp);
    CHECK(decode_key("\x1b[6~").key == kPageDown);
    CHECK(decode_key("\x1b[6~").used == 4);
}

TEST_CASE("one key at a time, even when several arrived together") {
    std::string in = "\x1b[Ax";
    KeyPress k = decode_key(in);
    CHECK(k.key == kArrowUp);
    CHECK(k.used == 3);
    CHECK(decode_key(std::string_view(in).substr(k.used)).key == 'x');
}

TEST_CASE("a lone ESC, or an unfinished sequence, is Escape") {
    CHECK(decode_key("\x1b").key == kEsc);
    CHECK(decode_key("\x1b").used == 1);
    CHECK(decode_key("\x1b[").key == kEsc);
    CHECK(decode_key("\x1b[").used == 1);
    CHECK(decode_key("\x1b[5").key == kEsc);
    CHECK(decode_key("\x1bx").key == kEsc);  // Alt-x
    CHECK(decode_key("\x1bx").used == 1);
}

TEST_CASE("sequences the editor doesn't use are swallowed whole") {
    // Ctrl-Right, F5, and a mouse-ish sequence: none should leak into the
    // text as stray characters.
    CHECK(decode_key("\x1b[1;5C").key == kUnknown);
    CHECK(decode_key("\x1b[1;5C").used == 6);
    CHECK(decode_key("\x1b[15~").key == kUnknown);
    CHECK(decode_key("\x1b[15~").used == 5);
    CHECK(decode_key("\x1bOP").key == kUnknown);  // F1
    CHECK(decode_key("\x1bOP").used == 3);
    CHECK(decode_key("\x1b[Zrest").used == 3);
}

TEST_CASE("raw mode: every flag, and nothing else changed") {
    termios t{};
    t.c_iflag = BRKINT | ICRNL | INPCK | ISTRIP | IXON | IGNPAR;
    t.c_oflag = OPOST;
    t.c_cflag = 0;
    t.c_lflag = ECHO | ICANON | IEXTEN | ISIG | ECHOE;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    termios raw = raw_attrs(t);
    CHECK(raw.c_iflag == IGNPAR);
    CHECK(raw.c_oflag == 0);
    CHECK((raw.c_cflag & CS8) == CS8);
    CHECK(raw.c_lflag == ECHOE);
    CHECK(raw.c_cc[VMIN] == 0);
    CHECK(raw.c_cc[VTIME] == 1);
}
