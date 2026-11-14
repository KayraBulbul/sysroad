#include "doctest.h"

#include "kite_pty.hpp"
#include "session.hpp"

namespace ktest {

TEST_CASE("an empty editor: tildes, with the welcome a third of the way down") {
    Kite k(8, 40);
    k.draw();
    CHECK(k.rows(0, 6) == "~\n"
                          "~\n"
                          "~      kite -- a tiny text editor\n"
                          "~\n"
                          "~\n"
                          "~\n");
    CHECK(k.cursor() == "row 0, col 0");
    CHECK(k.term.cursor_visible());
}

TEST_CASE("a file's lines, then tildes, with the cursor at the top left") {
    Kite k(8, 20);
    k.open("hello.txt", "hello\nworld\n");
    CHECK(k.rows(0, 4) == "hello\n"
                          "world\n"
                          "~\n"
                          "~\n");
    CHECK(k.cursor() == "row 0, col 0");
    CHECK_MESSAGE(k.term.cursor_visible(), "the cursor should be showing after the frame");
}

TEST_CASE("the tildes run all the way to the bottom row") {
    Kite k(4, 20);
    k.open("one.txt", "one line\n");
    CHECK(k.screen() == "one line\n"
                        "~\n"
                        "~\n"
                        "~\n");
}

TEST_CASE("kite FILE shows the file on the alternate screen, and Ctrl-Q brings the shell's screen back") {
    write_file("fog.txt", "The fog comes\non little cat feet.\n");
    KitePty kite(10, 30, {"fog.txt"}, {"$ kite fog.txt"});
    bool shown = kite.wait_until([](const Term& t) { return t.line(1) == "on little cat feet."; });
    REQUIRE_MESSAGE(shown, (std::string("kite should show fog.txt. ") + kite.shown()));
    CHECK_MESSAGE(kite.term().on_alt_screen(), "kite should draw on the alternate screen (\\x1b[?1049h)");
    CHECK(kite.term().line(0) == "The fog comes");
    CHECK(kite.term().line(2) == "~");
    kite.type("\x11");
    int status = kite.wait_exit();
    CHECK_MESSAGE(status == 0, kite.shown());
    CHECK_MESSAGE(!kite.term().on_alt_screen(), "kite should leave the alternate screen as it ends");
    CHECK_MESSAGE(kite.term().line(0) == "$ kite fog.txt", (std::string("the shell's screen should be back. ") + kite.shown()));
}

}  // namespace ktest
