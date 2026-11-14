#include "doctest.h"

#include <string>

#include "kite_pty.hpp"
#include "session.hpp"

namespace ktest {

TEST_CASE("no file: [No Name]") {
    Kite k(6, 25);
    k.draw();
    CHECK(k.line(4) == "[No Name] - 0 lines   1/0");
    CHECK(k.line(5) == "");
}

TEST_CASE("long names are cut to 20 characters") {
    Kite k(5, 50);
    k.open("a_very_long_file_name_indeed.txt", "a\n");
    CHECK(k.line(3) == "a_very_long_file_nam - 1 lines                 1/1");
}

TEST_CASE("a narrow window drops the right side, then cuts the left") {
    for (auto [cols, bar] : {std::pair{22, "x.txt - 1 lines    1/1"}, std::pair{18, "x.txt - 1 lines1/1"},
                             std::pair{17, "x.txt - 1 lines"}, std::pair{10, "x.txt - 1"}}) {
        CAPTURE(cols);
        Kite k(5, cols);
        k.open("x.txt", "a\n");
        CHECK(k.line(3) == bar);
        CHECK(k.inverted(3) == std::string(static_cast<std::size_t>(cols), '#'));
    }
}

TEST_CASE("the welcome is a third of the way down the text's rows") {
    Kite k(11, 40);  // 9 rows of text
    k.draw();
    CHECK(k.line(3) == "~      kite -- a tiny text editor");
    CHECK(k.line(2) == "~");
}

TEST_CASE("tildes stop above the bars") {
    Kite k(5, 20);
    k.open("short.txt", "a\n");
    CHECK(k.rows(0, 3) == "a\n~\n~\n");
    CHECK(k.line(3) == "short.txt - 1 lines");
}

TEST_CASE("the message is cut at the window's width") {
    Kite k(5, 5);
    open_file(k.e, "n.txt");
    k.draw();
    CHECK(k.line(4) == "New f");
    CHECK(k.inverted(4) == ".....");
}

TEST_CASE("on a terminal, the bars are its last two rows") {
    write_file("fog.txt", "The fog comes\non little cat feet.\n");
    KitePty kite(8, 40, {"fog.txt"});
    bool shown = kite.wait_until([](const Term& t) { return t.line(6).rfind("fog.txt - 2 lines", 0) == 0; });
    CHECK_MESSAGE(shown, (std::string("expected the status bar on row 6 of 8. ") + kite.shown()));
    kite.type("\x11");
    CHECK(kite.wait_exit() == 0);
}

}  // namespace ktest
