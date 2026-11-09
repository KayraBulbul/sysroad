#include "doctest.h"

#include <sstream>
#include <string>

#include "editor.hpp"

static Editor sized(int rows, int cols) {
    Editor e;
    e.screen_rows = rows;
    e.screen_cols = cols;
    return e;
}

TEST_CASE("Windows line endings, a missing last newline, and empty lines") {
    std::istringstream in("a\r\n\r\nb\n\nlast");
    auto rows = read_rows(in);
    REQUIRE(rows.size() == 5);
    CHECK(rows[0].chars == "a");
    CHECK(rows[1].chars == "");
    CHECK(rows[2].chars == "b");
    CHECK(rows[3].chars == "");
    CHECK(rows[4].chars == "last");
}

TEST_CASE("an empty file has no rows") {
    std::istringstream in("");
    CHECK(read_rows(in).empty());
}

TEST_CASE("read_rows fills in render") {
    std::istringstream in("\tindented\n");
    auto rows = read_rows(in);
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].render == "        indented");
}

TEST_CASE("tabs after text go to the next stop, and a full stop still moves") {
    Row r{"ab\tc\t\td", ""};
    update_render(r);
    CHECK(r.render == "ab      c               d");
    Row full{"12345678\tx", ""};
    update_render(full);
    CHECK(full.render == "12345678        x");
    CHECK(cx_to_rx(r, 0) == 0);
    CHECK(cx_to_rx(r, 2) == 2);
    CHECK(cx_to_rx(r, 3) == 8);
    CHECK(cx_to_rx(r, 4) == 9);
    CHECK(cx_to_rx(r, 5) == 16);
    CHECK(cx_to_rx(r, 6) == 24);
    CHECK(cx_to_rx(r, 7) == 25);
}

TEST_CASE("no tabs: render is chars, and rx is cx") {
    Row r{"plain text", ""};
    update_render(r);
    CHECK(r.render == "plain text");
    CHECK(cx_to_rx(r, 6) == 6);
}

TEST_CASE("scroll back up and left") {
    Editor e = sized(3, 5);
    e.rows = std::vector<Row>(10, Row{"0123456789", "0123456789"});
    e.rowoff = 6;
    e.coloff = 4;
    e.cy = 2;
    e.cx = 1;
    scroll(e);
    CHECK(e.rowoff == 2);
    CHECK(e.coloff == 1);
    CHECK(e.rx == 1);
}

TEST_CASE("scroll right by render columns, so tabs count") {
    Editor e = sized(3, 10);
    Row r{"\t\tx", ""};
    update_render(r);
    e.rows = {r};
    e.cx = 2;  // on the x, at render column 16
    scroll(e);
    CHECK(e.rx == 16);
    CHECK(e.coloff == 7);
}

TEST_CASE("the cursor on screen leaves the offsets alone") {
    Editor e = sized(5, 20);
    e.rows = std::vector<Row>(10, Row{"x", "x"});
    e.rowoff = 2;
    e.cy = 4;
    scroll(e);
    CHECK(e.rowoff == 2);
    CHECK(e.coloff == 0);
}

TEST_CASE("the line past the end has rx 0, and can be scrolled to") {
    Editor e = sized(2, 20);
    e.rows = std::vector<Row>(3, Row{"abc", "abc"});
    e.cy = 3;
    e.cx = 0;
    scroll(e);
    CHECK(e.rx == 0);
    CHECK(e.rowoff == 2);
}
