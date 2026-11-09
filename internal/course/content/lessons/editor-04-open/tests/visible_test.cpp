#include "doctest.h"

#include <sstream>

#include "editor.hpp"

TEST_CASE("read_rows splits lines and drops the newlines") {
    std::istringstream in("one\ntwo\n");
    auto rows = read_rows(in);
    REQUIRE(rows.size() == 2);
    CHECK(rows[0].chars == "one");
    CHECK(rows[1].chars == "two");
    CHECK(rows[1].render == "two");
}

TEST_CASE("a tab runs to the next multiple of 8 columns") {
    Row r{"\tx", ""};
    update_render(r);
    CHECK(r.render == "        x");
    CHECK(cx_to_rx(r, 1) == 8);
}

TEST_CASE("scrolling down follows the cursor") {
    Editor e;
    e.screen_rows = 3;
    e.rows = std::vector<Row>(10, Row{"x", "x"});
    e.cy = 5;
    scroll(e);
    CHECK(e.rowoff == 3);
}
