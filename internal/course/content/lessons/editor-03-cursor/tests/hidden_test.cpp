#include "doctest.h"

#include "keys.hpp"
#include "text.hpp"

TEST_CASE("left at the start of a line goes to the end of the one above") {
    Editor e = with_lines({"abc", "de"});
    e.cy = 1;
    press(e, kArrowLeft);
    CHECK(e.cy == 0);
    CHECK(e.cx == 3);
}

TEST_CASE("right at the end of a line goes to the start of the next") {
    Editor e = with_lines({"abc", "de"});
    press(e, kEnd);
    press(e, kArrowRight);
    CHECK(e.cy == 1);
    CHECK(e.cx == 0);
}

TEST_CASE("down stops on the line just past the end, where the cursor is at 0") {
    Editor e = with_lines({"abc", "de"});
    press(e, kArrowDown, 5);
    CHECK(e.cy == 2);
    CHECK(e.cx == 0);
    press(e, kArrowRight);  // nothing to the right of nothing
    CHECK(e.cy == 2);
    CHECK(e.cx == 0);
    press(e, kEnd);
    CHECK(e.cx == 0);
}

TEST_CASE("right at the end of the last line goes to the line past the end") {
    Editor e = with_lines({"ab"});
    press(e, kArrowRight, 3);
    CHECK(e.cy == 1);
    CHECK(e.cx == 0);
}

TEST_CASE("moving onto a shorter line snaps to its end") {
    Editor e = with_lines({"a long line", "short", "another long line"});
    press(e, kEnd);
    press(e, kArrowDown);
    CHECK(e.cx == 5);
    press(e, kArrowDown);
    CHECK(e.cx == 5);  // it doesn't remember the column it came from
}

TEST_CASE("Page Down and Page Up move a screenful") {
    Editor e = with_lines({"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"}, 4);
    press(e, kPageDown);
    CHECK(e.cy == 4);
    press(e, kPageDown, 2);
    CHECK(e.cy == 10);  // stops on the line past the end
    press(e, kPageUp);
    CHECK(e.cy == 6);
    press(e, kPageUp, 2);
    CHECK(e.cy == 0);
}

TEST_CASE("other keys don't move it, and an empty file has one place to be") {
    Editor e = with_lines({});
    for (int key :
         {static_cast<int>('x'), static_cast<int>(kArrowDown), static_cast<int>(kArrowRight),
          static_cast<int>(kEnd), static_cast<int>(kPageDown)}) {
        press(e, key);
        CHECK(e.cx == 0);
        CHECK(e.cy == 0);
    }
}
