#include "doctest.h"

#include "text.hpp"

TEST_CASE("typing inserts at the cursor and moves it on") {
    Editor e = with_lines({"hed"});
    e.cx = 2;
    type(e, "llo worl");
    CHECK(text(e) == "hello world");
    CHECK(e.cx == 10);
}

TEST_CASE("Enter splits the line") {
    Editor e = with_lines({"helloworld"});
    e.cx = 5;
    insert_newline(e);
    CHECK(text(e) == "hello|world");
    CHECK(e.cy == 1);
    CHECK(e.cx == 0);
}

TEST_CASE("Backspace deletes the character before the cursor") {
    Editor e = with_lines({"abc"});
    e.cx = 2;
    delete_char(e);
    CHECK(text(e) == "ac");
    CHECK(e.cx == 1);
}
