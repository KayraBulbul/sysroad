#include "doctest.h"

#include "keys.hpp"
#include "text.hpp"

TEST_CASE("arrows move one step") {
    Editor e = with_lines({"hello", "world"});
    press(e, kArrowRight, 3);
    CHECK(e.cx == 3);
    press(e, kArrowDown);
    CHECK(e.cy == 1);
    press(e, kArrowLeft);
    CHECK(e.cx == 2);
    press(e, kArrowUp);
    CHECK(e.cy == 0);
}

TEST_CASE("the cursor stops at the top left") {
    Editor e = with_lines({"hello"});
    press(e, kArrowUp);
    press(e, kArrowLeft);
    CHECK(e.cx == 0);
    CHECK(e.cy == 0);
}

TEST_CASE("Home and End") {
    Editor e = with_lines({"hello"});
    press(e, kEnd);
    CHECK(e.cx == 5);
    press(e, kHome);
    CHECK(e.cx == 0);
}
