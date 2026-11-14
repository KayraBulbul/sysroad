#include "doctest.h"

#include "session.hpp"

namespace ktest {

TEST_CASE("an edit marks the file modified in the status bar") {
    Kite k(6, 40);
    k.open("notes.txt", "one\n");
    CHECK(k.line(4) == "notes.txt - 1 lines                  1/1");
    k.type("x");
    CHECK(k.line(4) == "notes.txt - 1 lines (modified)       1/1");
}

TEST_CASE("Ctrl-S writes every line with a newline, clears modified, and says how much it wrote") {
    Kite k(6, 40);
    k.open("saved.txt", "one\n\nthree\n");
    k.press(keys::end).type("!").press(keys::ctrl_s);
    CHECK(read_file("saved.txt") == "one!\n\nthree\n");
    CHECK(k.line(4) == "saved.txt - 3 lines                  1/3");
    CHECK(k.line(5) == "12 bytes written to disk");
}

TEST_CASE("Ctrl-Q with unsaved changes warns first") {
    Kite k(6, 70);
    k.open("q.txt", "q\n");
    k.type("x").press(keys::ctrl_q);
    CHECK_FALSE(k.quit);
    CHECK(k.line(5) == "Unsaved changes! Press Ctrl-Q 3 more times to quit anyway.");
}

}  // namespace ktest
