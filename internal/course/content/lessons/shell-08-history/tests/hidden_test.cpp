#include "doctest.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

#include "keys.hpp"
#include "lineedit.hpp"
#include "shell.hpp"

static void type(LineEditor& e, const std::string& s) {
    for (char c : s) {
        e.feed(static_cast<unsigned char>(c));
    }
}

TEST_CASE("Home, End, Delete, and the edges") {
    std::vector<std::string> h;
    LineEditor e(h);
    e.feed(kBackspace);
    e.feed(kArrowLeft);
    e.feed(kDelete);
    CHECK(e.text().empty());
    type(e, "abc");
    e.feed(kHome);
    CHECK(e.cursor() == 0);
    e.feed(kDelete);
    CHECK(e.text() == "bc");
    e.feed(ctrl_key('e'));
    CHECK(e.cursor() == 2);
    e.feed(kArrowRight);
    CHECK(e.cursor() == 2);
    e.feed(ctrl_key('a'));
    type(e, "x");
    CHECK(e.text() == "xbc");
    e.feed(kEnd);
    e.feed(kDelete);
    CHECK(e.text() == "xbc");
}

TEST_CASE("Ctrl-U, Ctrl-K and Ctrl-W") {
    std::vector<std::string> h;
    LineEditor e(h);
    type(e, "git commit -m");
    e.feed(ctrl_key('w'));
    CHECK(e.text() == "git commit ");
    e.feed(ctrl_key('w'));
    CHECK(e.text() == "git ");
    type(e, "log --oneline");
    for (int i = 0; i < 10; ++i) {
        e.feed(kArrowLeft);
    }
    e.feed(ctrl_key('k'));
    CHECK(e.text() == "git log");
    e.feed(kArrowLeft);
    e.feed(kArrowLeft);
    e.feed(kArrowLeft);
    e.feed(ctrl_key('u'));
    CHECK(e.text() == "log");
    CHECK(e.cursor() == 0);
}

TEST_CASE("Ctrl-C cancels, Ctrl-D ends input only on an empty line") {
    std::vector<std::string> h;
    LineEditor e(h);
    CHECK(e.feed(ctrl_key('d')) == LineEditor::EndOfInput);
    type(e, "ab");
    e.feed(kHome);
    CHECK(e.feed(ctrl_key('d')) == LineEditor::Editing);  // deletes instead
    CHECK(e.text() == "b");
    CHECK(e.feed(ctrl_key('c')) == LineEditor::Cancelled);
}

TEST_CASE("keys that aren't text are ignored") {
    std::vector<std::string> h;
    LineEditor e(h);
    e.feed(kUnknown);
    e.feed(kPageUp);
    e.feed(kEsc);
    e.feed(ctrl_key('g'));
    CHECK(e.text().empty());
    CHECK(e.feed(kEnter) == LineEditor::Done);
}

TEST_CASE("history browsing: limits, and an edited entry") {
    std::vector<std::string> h{"one", "two"};
    LineEditor e(h);
    e.feed(kArrowDown);  // not browsing: nothing happens
    CHECK(e.text().empty());
    e.feed(kArrowUp);
    e.feed(kArrowUp);
    e.feed(kArrowUp);  // past the oldest: stays
    CHECK(e.text() == "one");
    CHECK(e.cursor() == 3);
    std::vector<std::string> none;
    LineEditor empty(none);
    empty.feed(kArrowUp);
    CHECK(empty.text().empty());
}

TEST_CASE("render") {
    std::vector<std::string> h;
    LineEditor e(h);
    type(e, "abc");
    CHECK(e.render("$ ") == "\r$ abc\x1b[K");
    e.feed(kArrowLeft);
    e.feed(kArrowLeft);
    CHECK(e.render("$ ") == "\r$ abc\x1b[K\x1b[2D");
}

TEST_CASE("add_history skips blanks and repeats, and keeps 1000") {
    Shell sh;
    add_history(sh, "");
    add_history(sh, "   ");
    add_history(sh, "ls");
    add_history(sh, "ls");
    add_history(sh, "pwd");
    add_history(sh, "ls");
    CHECK(sh.history == std::vector<std::string>{"ls", "pwd", "ls"});
    for (int i = 0; i < 1100; ++i) {
        add_history(sh, "cmd " + std::to_string(i));
    }
    CHECK(sh.history.size() == 1000);
    CHECK(sh.history.back() == "cmd 1099");
    CHECK(sh.history.front() == "cmd 100");
}

TEST_CASE("the history builtin numbers from 1, and takes a count") {
    Shell sh;
    add_history(sh, "ls");
    add_history(sh, "pwd");
    add_history(sh, "history");
    std::fflush(nullptr);
    int saved = dup(1);
    int f = open("hist-out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(f, 1);
    close(f);
    find_builtin("history")(sh, {"history"});
    find_builtin("history")(sh, {"history", "2"});
    std::cout.flush();
    std::fflush(nullptr);
    dup2(saved, 1);
    close(saved);
    std::ifstream in("hist-out.txt");
    std::stringstream ss;
    ss << in.rdbuf();
    CHECK(ss.str() == "    1  ls\n    2  pwd\n    3  history\n    2  pwd\n    3  history\n");
}

TEST_CASE("save and load") {
    Shell a;
    add_history(a, "echo one");
    add_history(a, "echo 'two words'");
    save_history(a, "hist.txt");
    Shell b;
    add_history(b, "already here");
    load_history(b, "hist.txt");
    CHECK(b.history == std::vector<std::string>{"already here", "echo one", "echo 'two words'"});
    Shell c;
    load_history(c, "no-such-history-file");
    CHECK(c.history.empty());
}

TEST_CASE("only the owner can read the history file") {
    Shell a;
    add_history(a, "export TOKEN=hunter2");
    {
        std::ofstream("private.txt.tmp") << "left over";
    }
    chmod("private.txt.tmp", 0666);
    save_history(a, "private.txt");
    struct stat st{};
    REQUIRE(stat("private.txt", &st) == 0);
    CHECK((st.st_mode & 0777) == 0600);
}
