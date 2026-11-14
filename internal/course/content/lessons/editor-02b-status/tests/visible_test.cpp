#include "doctest.h"

#include <string>

#include "session.hpp"

namespace ktest {

TEST_CASE("the status bar: the name, the lines and the cursor's line, inverted across the width") {
    Kite k(6, 30);
    k.open("notes.txt", "one\ntwo\n");
    CHECK(k.line(4) == "notes.txt - 2 lines        1/2");
    CHECK_MESSAGE(k.inverted(4) == std::string(30, '#'),
                  "the whole status bar row should be inverted, padded with spaces to the edge");
    CHECK(k.inverted(0) == std::string(30, '.'));
}

TEST_CASE("the text gets the rows above the two bars") {
    Kite k(5, 30);
    k.open("five.txt", "1\n2\n3\n4\n5\n");
    CHECK(k.screen() == "1\n"
                        "2\n"
                        "3\n"
                        "five.txt - 5 lines         1/5\n"
                        "\n");
}

TEST_CASE("a new file: its name in the status bar, and New file in the message bar") {
    Kite k(9, 40);
    open_file(k.e, "draft.txt");
    k.draw();
    CHECK(k.screen() == "~\n"
                        "~\n"
                        "~      kite -- a tiny text editor\n"
                        "~\n"
                        "~\n"
                        "~\n"
                        "~\n"
                        "draft.txt - 0 lines                  1/0\n"
                        "New file\n");
}

}  // namespace ktest
