#include "doctest.h"

#include "text.hpp"

TEST_CASE("each word is a step: a space ends it") {
    Editor e = with_lines({""});
    type(e, "hello world");
    REQUIRE(e.history.undo.size() == 2);
    undo(e);
    CHECK(text(e) == "hello  @0:6");
    undo(e);
    CHECK(text(e) == " @0:0");
    redo(e);
    redo(e);
    CHECK(text(e) == "hello world @0:11");
}

TEST_CASE("Backspaces join into one step, and undo puts the cursor back") {
    Editor e = with_lines({"abcdef"});
    e.cx = 5;
    backspace(e, 3);
    CHECK(text(e) == "abf @0:2");
    REQUIRE(e.history.undo.size() == 1);
    CHECK(e.history.undo[0].text == "cde");
    undo(e);
    CHECK(text(e) == "abcdef @0:5");
    redo(e);
    CHECK(text(e) == "abf @0:2");
}

TEST_CASE("Deletes join too, growing to the right") {
    Editor e = with_lines({"abcdef"});
    e.cx = 1;
    delete_forward(e);
    delete_forward(e);
    CHECK(text(e) == "adef @0:1");
    REQUIRE(e.history.undo.size() == 1);
    CHECK(e.history.undo[0].text == "bc");
    undo(e);
    CHECK(text(e) == "abcdef @0:1");
}

TEST_CASE("Enter is a step of its own; undoing it joins the lines") {
    Editor e = with_lines({"onetwo"});
    e.cx = 3;
    type(e, "\n");
    type(e, "x");
    CHECK(text(e) == "one|xtwo @1:1");
    REQUIRE(e.history.undo.size() == 2);
    undo(e);
    CHECK(text(e) == "one|two @1:0");
    undo(e);
    CHECK(text(e) == "onetwo @0:3");
    redo(e);
    CHECK(text(e) == "one|two @1:0");
}

TEST_CASE("Backspace across a line break, undone") {
    Editor e = with_lines({"ab", "cd"});
    e.cy = 1;
    backspace(e, 2);
    CHECK(text(e) == "acd @0:1");
    REQUIRE(e.history.undo.size() == 2);  // the join is a step on its own
    undo(e);
    CHECK(text(e) == "abcd @0:2");
    undo(e);
    CHECK(text(e) == "ab|cd @1:0");
}

TEST_CASE("a new edit after undo empties redo") {
    Editor e = with_lines({""});
    type(e, "abc");
    undo(e);
    type(e, "x");
    CHECK_FALSE(redo(e));
    CHECK(text(e) == "x @0:1");
}

TEST_CASE("typing after an undo starts a new step") {
    Editor e = with_lines({""});
    type(e, "ab ");
    type(e, "cd");
    undo(e);  // takes "cd"
    type(e, "x");
    REQUIRE(e.history.undo.size() == 2);
    CHECK(e.history.undo.back().text == "x");
}

TEST_CASE("sealed: typing elsewhere, or after moving, is a new step") {
    History h;
    record(h, Edit{Edit::Kind::Insert, {0, 0}, "a", {0, 0}});
    h.sealed = true;
    record(h, Edit{Edit::Kind::Insert, {0, 1}, "b", {0, 1}});
    CHECK(h.undo.size() == 2);
    CHECK_FALSE(h.sealed);
    record(h, Edit{Edit::Kind::Insert, {0, 5}, "c", {0, 5}});  // not where "b" ended
    CHECK(h.undo.size() == 3);
    record(h, Edit{Edit::Kind::Insert, {1, 6}, "d", {1, 6}});  // another line
    CHECK(h.undo.size() == 4);
}

TEST_CASE("typing and erasing don't join, nor do edits with line breaks") {
    History h;
    record(h, Edit{Edit::Kind::Insert, {0, 0}, "ab", {0, 0}});
    record(h, Edit{Edit::Kind::Erase, {0, 1}, "b", {0, 2}});
    CHECK(h.undo.size() == 2);
    record(h, Edit{Edit::Kind::Insert, {0, 1}, "\n", {0, 1}});
    record(h, Edit{Edit::Kind::Insert, {1, 0}, "x", {1, 0}});
    CHECK(h.undo.size() == 4);
}

TEST_CASE("undo and redo many steps, all the way and back") {
    Editor e = with_lines({"start"});
    e.cx = 5;
    type(e, " one\ntwo three");
    std::string done = text(e);
    int steps = 0;
    while (undo(e)) {
        ++steps;
    }
    CHECK(text(e) == "start @0:5");
    CHECK(steps == 5);  // " ", "one", "\n", "two ", "three"
    while (redo(e)) {
    }
    CHECK(text(e) == done);
}

TEST_CASE("undo marks the file changed, and keeps the colours right") {
    Editor e = with_lines({"x"});
    e.highlight = true;
    update_syntax(e, 0);
    e.cx = 1;
    type(e, " // hi");
    e.dirty = false;
    while (undo(e)) {
    }
    CHECK(e.dirty);
    CHECK(e.rows[0].chars == "x");
    CHECK(e.rows[0].hl.size() == 1);
    CHECK(e.rows[0].hl[0] == Hl::Normal);
}
