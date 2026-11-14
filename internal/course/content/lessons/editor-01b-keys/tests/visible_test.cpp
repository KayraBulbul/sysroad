#include "doctest.h"

#include "keys.hpp"
#include "kite_pty.hpp"

namespace ktest {

TEST_CASE("an ordinary byte is a key by itself, one byte at a time") {
    CHECK(key("a") == 'a');
    CHECK(used("a") == 1);
    CHECK(key("\r") == 13);   // Enter
    CHECK(key("\x11") == 17);  // Ctrl-Q
    CHECK(used("xyz") == 1);
}

TEST_CASE("the arrows are ESC [ A to D: four keys, none of them a byte") {
    for (auto [name, arrow] : {std::pair{"Up", "\x1b[A"}, std::pair{"Down", "\x1b[B"}, std::pair{"Right", "\x1b[C"},
                               std::pair{"Left", "\x1b[D"}}) {
        INFO((std::string("the ") + name + " key"));
        CHECK_MESSAGE(key(arrow) > 255, "special keys have values above 255");
        CHECK(used(arrow) == 3);
    }
    CHECK(key("\x1b[A") != key("\x1b[B"));
    CHECK(key("\x1b[A") != key("\x1b[C"));
    CHECK(key("\x1b[A") != key("\x1b[D"));
    CHECK(key("\x1b[B") != key("\x1b[C"));
    CHECK(key("\x1b[B") != key("\x1b[D"));
    CHECK(key("\x1b[C") != key("\x1b[D"));
}

TEST_CASE("kite prints the name of each key you press") {
    KitePty kite(12, 40);
    REQUIRE(kite.wait_for_raw_mode());
    kite.type("x");
    kite.type("\x1b[A");
    kite.type(" ");
    kite.type("\x11");
    int status = kite.wait_exit();
    CHECK_MESSAGE(status == 0, (std::string("Ctrl-Q should end kite with exit status 0. ") + kite.shown()));
    CHECK_MESSAGE(contains(kite.term().screen(), "x\nUp\nSpace\nCtrl-Q\n"), (std::string("expected the lines x, Up, Space and Ctrl-Q. ") + kite.shown()));
}

}  // namespace ktest
