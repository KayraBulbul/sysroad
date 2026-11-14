#include "doctest.h"

#include "kite.hpp"
#include "kite_pty.hpp"

namespace ktest {

TEST_CASE("raw_attrs switches off echo and line buffering") {
    termios t{};
    t.c_lflag = ECHO | ICANON;
    termios raw = raw_attrs(t);
    bool echo_on = (raw.c_lflag & ECHO) != 0;
    bool line_by_line = (raw.c_lflag & ICANON) != 0;
    CHECK_FALSE_MESSAGE(echo_on, "ECHO is still on in c_lflag");
    CHECK_FALSE_MESSAGE(line_by_line, "ICANON is still on in c_lflag");
}

TEST_CASE("kite puts the terminal in raw mode, and Ctrl-Q puts it back") {
    KitePty kite(10, 40);
    REQUIRE_MESSAGE(kite.wait_for_raw_mode(), "the terminal still echoes: kite didn't switch on raw mode");
    CHECK_MESSAGE((kite.modes().c_lflag & ICANON) == 0, "raw mode, but still line by line (ICANON)");
    kite.type("\x11");  // Ctrl-Q
    int status = kite.wait_exit();
    CHECK_MESSAGE(status == 0, (std::string("Ctrl-Q should end kite with exit status 0. ") + kite.shown()));
    termios after = kite.modes();
    CHECK_MESSAGE((after.c_lflag & ECHO) != 0, "after kite ends, the terminal should echo again");
    CHECK_MESSAGE((after.c_lflag & ICANON) != 0, "after kite ends, the terminal should be line by line again");
}

}  // namespace ktest
