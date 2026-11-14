#include "doctest.h"

#include <unistd.h>

#include "keys.hpp"
#include "kite_pty.hpp"

namespace ktest {

TEST_CASE("bytes above 127 aren't negative") {
    CHECK(key("\xc3\xa9") == 0xc3);
    CHECK(key("\xff") == 255);
}

TEST_CASE("Home and End have four spellings each") {
    CHECK(key("\x1bOH") == key("\x1b[H"));
    CHECK(key("\x1b[1~") == key("\x1b[H"));
    CHECK(key("\x1b[7~") == key("\x1b[H"));
    CHECK(key("\x1bOF") == key("\x1b[F"));
    CHECK(key("\x1b[4~") == key("\x1b[F"));
    CHECK(key("\x1b[8~") == key("\x1b[F"));
    CHECK(used("\x1bOH") == 3);
    CHECK(used("\x1b[6~") == 4);
}

TEST_CASE("the nine special keys are nine different keys, none of them a byte") {
    for (std::size_t i = 0; i < kSpecialKeys.size(); ++i) {
        INFO(kSpecialNames[i]);
        CHECK(key(kSpecialKeys[i]) > 255);
        for (std::size_t j = 0; j < i; ++j) {
            INFO((std::string("isn't ") + kSpecialNames[j]));
            CHECK(key(kSpecialKeys[i]) != key(kSpecialKeys[j]));
        }
    }
}

TEST_CASE("one key at a time, even when several arrived together") {
    std::string in = "\x1b[Ax";
    auto [k, n] = decode_key(in);
    CHECK(k == key("\x1b[A"));
    CHECK(n == 3);
    CHECK(key(std::string_view(in).substr(n)) == 'x');
}

TEST_CASE("a lone ESC, or an unfinished sequence, is Escape") {
    CHECK(key("\x1b") == 27);
    CHECK(used("\x1b") == 1);
    CHECK(key("\x1b[") == 27);
    CHECK(used("\x1b[") == 1);
    CHECK(key("\x1b[5") == 27);
    CHECK(key("\x1bx") == 27);  // Alt-x
    CHECK(used("\x1bx") == 1);
}

TEST_CASE("sequences kite doesn't use are swallowed whole, as a key of their own") {
    // Ctrl-Right, F5, F1 and Shift-Tab: none should leak into the text as
    // stray characters, and none is one of the keys kite uses.
    for (auto [name, seq] : {std::pair{"Ctrl-Right", "\x1b[1;5C"}, std::pair{"F5", "\x1b[15~"},
                             std::pair{"F1", "\x1bOP"}, std::pair{"Shift-Tab", "\x1b[Z"}}) {
        INFO(name);
        CHECK(key(seq) > 255);
        for (std::size_t j = 0; j < kSpecialKeys.size(); ++j) {
            INFO((std::string("isn't ") + kSpecialNames[j]));
            CHECK(key(seq) != key(kSpecialKeys[j]));
        }
    }
    CHECK(used("\x1b[1;5C") == 6);
    CHECK(used("\x1b[15~") == 5);
    CHECK(used("\x1bOP") == 3);
    CHECK(used("\x1b[Zrest") == 3);
}

TEST_CASE("an arrow that arrives in two pieces is still one key") {
    KitePty kite(12, 40);
    REQUIRE(kite.wait_for_raw_mode());
    kite.type("\x1b");
    usleep(20'000);
    kite.type("[B");
    kite.type("\x11");
    int status = kite.wait_exit();
    CHECK_MESSAGE(status == 0, kite.shown());
    CHECK_MESSAGE(contains(kite.term().screen(), "Down\nCtrl-Q\n"), (std::string("expected Down, not Esc, [ and B. ") + kite.shown()));
}

TEST_CASE("Escape on its own is Escape, once nothing follows it for a moment") {
    KitePty kite(12, 40);
    REQUIRE(kite.wait_for_raw_mode());
    kite.type("\x1b");
    usleep(400'000);
    kite.type("[\x11");
    int status = kite.wait_exit();
    CHECK_MESSAGE(status == 0, kite.shown());
    CHECK_MESSAGE(contains(kite.term().screen(), "Esc\n[\nCtrl-Q\n"), (std::string("expected Esc, then [. ") + kite.shown()));
}

}  // namespace ktest
