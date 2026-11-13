#include "doctest.h"

#include "keys.hpp"
#include "lineedit.hpp"
#include "shell.hpp"

static void type(LineEditor& e, const std::string& s) {
    for (char c : s) {
        e.feed(static_cast<unsigned char>(c));
    }
}

TEST_CASE("typing, moving and deleting") {
    std::vector<std::string> history;
    LineEditor e(history);
    type(e, "echo hllo");
    for (int i = 0; i < 3; ++i) {
        e.feed(kArrowLeft);
    }
    type(e, "e");
    CHECK(e.text() == "echo hello");
    CHECK(e.cursor() == 7);
    e.feed(kBackspace);
    e.feed(kBackspace);
    CHECK(e.text() == "echo llo");
    CHECK(e.feed(kEnter) == LineEditor::Done);
}

TEST_CASE("Up and Down walk the history") {
    std::vector<std::string> history{"first", "second"};
    LineEditor e(history);
    type(e, "draft");
    e.feed(kArrowUp);
    CHECK(e.text() == "second");
    e.feed(kArrowUp);
    CHECK(e.text() == "first");
    e.feed(kArrowDown);
    CHECK(e.text() == "second");
    e.feed(kArrowDown);
    CHECK(e.text() == "draft");
}

TEST_CASE("history: add and list") {
    Shell sh;
    add_history(sh, "ls");
    add_history(sh, "pwd");
    CHECK(sh.history == std::vector<std::string>{"ls", "pwd"});
}
