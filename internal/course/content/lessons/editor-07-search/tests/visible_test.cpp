#include "doctest.h"

#include "search.hpp"

namespace ktest {

TEST_CASE("Ctrl-F asks in the message bar, and the cursor jumps to the first match as you type") {
    Kite k = cats();
    k.press(keys::ctrl_f).type("c");
    CHECK_MESSAGE(asking(k, "c"), (std::string("the message bar should start with Search and end with the query: ") + k.line(9)));
    CHECK(k.cursor() == "row 0, col 4");
    k.type("at");
    CHECK(asking(k, "cat"));
    CHECK(k.cursor() == "row 0, col 4");
}

TEST_CASE("the search starts at the cursor, and a match right there counts") {
    Kite k = cats();
    k.press(keys::down).press(keys::right, 2);
    k.press(keys::ctrl_f).type("cat");
    CHECK(k.cursor() == "row 1, col 2");
}

TEST_CASE("Enter stays at the match; Escape goes back to where the search started") {
    Kite k = cats();
    k.press(keys::ctrl_f).type("sat").press(keys::enter);
    CHECK(k.cursor() == "row 1, col 6");
    CHECK_FALSE(asking(k, ""));
    k.press(keys::ctrl_f).type("cat").press(keys::esc);
    CHECK(k.cursor() == "row 1, col 6");
}

}  // namespace ktest
