#include "doctest.h"

#include "kite_pty.hpp"
#include "session.hpp"

namespace ktest {

TEST_CASE("long lines are cut at the window's width, not wrapped") {
    Kite k(6, 4);
    k.open("long.txt", "abcdefgh\nxy\n");
    CHECK(k.rows(0, 3) == "abcd\n"
                          "xy\n"
                          "~\n");
}

TEST_CASE("a narrow window cuts the welcome too, with no room for a tilde") {
    Kite k(5, 10);
    k.draw();
    CHECK(k.line(1) == "kite -- a");
}

TEST_CASE("the welcome is centred, and an odd gap rounds down") {
    Kite k(8, 29);  // 3 spare columns: 1 on the left
    k.draw();
    CHECK(k.line(2) == "~kite -- a tiny text editor");
}

TEST_CASE("the welcome is only for an empty file") {
    Kite k(8, 40);
    k.open("x.txt", "x\n");
    CHECK(k.rows(0, 4) == "x\n~\n~\n~\n");
}

TEST_CASE("a file that isn't there opens empty, with the welcome") {
    Kite k(8, 40);
    open_file(k.e, "no-such-file.txt");
    k.draw();
    CHECK(k.line(2) == "~      kite -- a tiny text editor");
}

TEST_CASE("a last line without a newline counts, and so do empty lines") {
    Kite k(8, 20);
    k.open("lines.txt", "a\n\nb");
    CHECK(k.rows(0, 4) == "a\n\nb\n~\n");
}

TEST_CASE("more lines than fit: the first ones, and nothing scrolls the screen") {
    Kite k(4, 10);
    k.open("nine.txt", "1\n2\n3\n4\n5\n6\n7\n8\n9\n");
    CHECK_MESSAGE(k.line(0) == "1", "the top row should be the first line. A newline after the "
                                    "bottom row scrolls the whole screen up a line.");
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("the bottom row is the last line that fits") {
    Kite k(4, 10);
    k.open("nine.txt", "1\n2\n3\n4\n5\n6\n7\n8\n9\n");
    CHECK(k.screen() == "1\n2\n3\n4\n");
}

TEST_CASE("each frame replaces everything the last one left") {
    Kite k(6, 12);
    k.term.write("old old old old old old old old old old old old old old old old");
    k.open("new.txt", "new\n");
    CHECK(k.rows(0, 3) == "new\n~\n~\n");
}

TEST_CASE("a smaller window cuts the lines shorter") {
    Kite k(8, 20);
    k.open("wide.txt", "abcdefghijklmnop\n");
    set_window_size(k.e, 8, 5);
    Term small(8, 5);
    small.write(draw_screen(k.e));
    CHECK(small.line(0) == "abcde");
}

TEST_CASE("kite draws to the size of the terminal it's in") {
    write_file("wide.txt", "0123456789abcdefghij\nnext\n");
    KitePty kite(6, 12, {"wide.txt"});
    bool shown = kite.wait_until([](const Term& t) { return t.line(1) == "next"; });
    CHECK_MESSAGE(shown, (std::string("expected the lines cut at 12 columns. ") + kite.shown()));
    CHECK(kite.term().line(0) == "0123456789ab");
    kite.type("\x11");
    CHECK(kite.wait_exit() == 0);
}

}  // namespace ktest
