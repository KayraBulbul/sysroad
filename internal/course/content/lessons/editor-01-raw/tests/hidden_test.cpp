#include "doctest.h"

#include "kite.hpp"
#include "kite_pty.hpp"

namespace ktest {

TEST_CASE("raw_attrs: every flag, and nothing else changed") {
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

TEST_CASE("raw_attrs keeps the settings it isn't told to change") {
    termios t{};
    t.c_cflag = CREAD | CLOCAL;
    t.c_cc[VINTR] = 3;
    termios raw = raw_attrs(t);
    CHECK((raw.c_cflag & (CREAD | CLOCAL)) == (CREAD | CLOCAL));
    CHECK(raw.c_cc[VINTR] == 3);
}

TEST_CASE("Ctrl-C reaches kite as a key instead of ending it") {
    KitePty kite(10, 40);
    REQUIRE(kite.wait_for_raw_mode());
    kite.type("\x03");  // Ctrl-C: with ISIG on, the terminal would send SIGINT
    kite.type("\x11");
    int status = kite.wait_exit();
    CHECK_MESSAGE(status == 0, (std::string("kite should still be running after Ctrl-C, and quit on Ctrl-Q. ") + kite.shown()));
}

TEST_CASE("while kite runs, Ctrl-S and Enter reach it as they are, and output isn't changed") {
    KitePty kite(10, 40);
    REQUIRE(kite.wait_for_raw_mode());
    termios t = kite.modes();
    CHECK_MESSAGE((t.c_iflag & IXON) == 0, "IXON is on: Ctrl-S would freeze the terminal");
    CHECK_MESSAGE((t.c_iflag & ICRNL) == 0, "ICRNL is on: Enter would arrive as \\n");
    CHECK_MESSAGE((t.c_oflag & OPOST) == 0, "OPOST is on: the terminal would change what kite writes");
    kite.type("\x11");
    int status = kite.wait_exit();
    CHECK_MESSAGE(status == 0, (std::string("Ctrl-Q should end kite with exit status 0. ") + kite.shown()));
}

}  // namespace ktest
