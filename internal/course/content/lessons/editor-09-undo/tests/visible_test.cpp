#include "doctest.h"

#include "text.hpp"

TEST_CASE("a typed word undoes in one step, and redoes") {
    Editor e = with_lines({""});
    type(e, "hello");
    CHECK(undo(e));
    CHECK(text(e) == " @0:0");
    CHECK(redo(e));
    CHECK(text(e) == "hello @0:5");
}

TEST_CASE("nothing to undo or redo") {
    Editor e = with_lines({"x"});
    CHECK_FALSE(undo(e));
    CHECK_FALSE(redo(e));
    CHECK(text(e) == "x @0:0");
}

TEST_CASE("record joins typing that carries on") {
    History h;
    record(h, Edit{Edit::Kind::Insert, {0, 0}, "a", {0, 0}});
    record(h, Edit{Edit::Kind::Insert, {0, 1}, "b", {0, 1}});
    REQUIRE(h.undo.size() == 1);
    CHECK(h.undo[0].text == "ab");
}
