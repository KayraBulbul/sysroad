#include "doctest.h"

#include "search.hpp"

namespace ktest {

TEST_CASE("forwards wraps around to the top") {
    Kite k(10, 60);
    k.open("w.txt", "cat one\ntwo\nthree cat\n");
    k.press(keys::down, 2).press(keys::end);
    k.press(keys::ctrl_f).type("cat");
    CHECK(k.cursor() == "row 0, col 0");
    k.press(keys::esc).press(keys::down);  // the line past the end
    k.press(keys::ctrl_f).type("cat");
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("Right and Down step to the next match, Left and Up to the one before, wrapping around") {
    Kite k(10, 60);
    k.open("a.txt", "a.a\n\n..a\n");
    k.press(keys::ctrl_f).type("a");
    std::string seen;
    for (std::string_view key : {keys::right, keys::down, keys::right, keys::right}) {
        k.press(key);
        seen += k.cursor() + "; ";
    }
    CHECK(seen == "row 0, col 2; row 2, col 2; row 0, col 0; row 0, col 2; ");
    std::string back;
    for (std::string_view key : {keys::left, keys::up, keys::left}) {
        k.press(key);
        back += k.cursor() + "; ";
    }
    CHECK(back == "row 0, col 0; row 2, col 2; row 0, col 2; ");
    CHECK(asking(k, "a"));
}

TEST_CASE("several matches on a line, overlapping ones too") {
    Kite k(10, 60);
    k.open("ab.txt", "abababab\n");
    k.press(keys::right).press(keys::ctrl_f).type("ab");
    CHECK(k.cursor() == "row 0, col 2");
    k.press(keys::esc).press(keys::home).press(keys::ctrl_f).type("aba");
    CHECK(k.cursor() == "row 0, col 0");
    k.press(keys::right);
    CHECK(k.cursor() == "row 0, col 2");
    k.press(keys::right);
    CHECK(k.cursor() == "row 0, col 4");
    k.press(keys::right);
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("backwards from the first column wraps around to the last match") {
    Kite k(10, 60);
    k.open("cd.txt", "cat\ndog\ncat dog cat\n");
    k.press(keys::ctrl_f).type("cat").press(keys::left);
    CHECK(k.cursor() == "row 2, col 8");
    k.press(keys::left);
    CHECK(k.cursor() == "row 2, col 0");
}

TEST_CASE("no match: the cursor goes back, and Enter says Not found") {
    Kite k(10, 60);
    k.open("t.txt", "the cat\n");
    k.press(keys::right);
    k.press(keys::ctrl_f).type("c");
    CHECK(k.cursor() == "row 0, col 4");
    k.type("x");
    CHECK(k.cursor() == "row 0, col 1");
    k.press(keys::enter);
    CHECK(k.line(9) == "Not found");
    CHECK(k.cursor() == "row 0, col 1");
}

TEST_CASE("matches are exact, case and all, and stay within a line") {
    Kite k(10, 60);
    k.open("c.txt", "ca\nt\nCat\n");
    k.press(keys::ctrl_f).type("cat");
    CHECK(k.cursor() == "row 0, col 0");
    k.press(keys::enter);
    CHECK(k.line(9) == "Not found");
    k.press(keys::ctrl_f).type("Cat");
    CHECK(k.cursor() == "row 2, col 0");
}

TEST_CASE("Backspace shortens the query, and an empty one matches nothing") {
    Kite k(10, 60);
    k.open("t.txt", "the cat\nthe dog\n");
    k.press(keys::ctrl_f).type("dog");
    CHECK(k.cursor() == "row 1, col 4");
    k.press(keys::backspace, 3);
    CHECK(asking(k, ""));
    CHECK(k.cursor() == "row 0, col 0");
    k.type("t");
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("the screen follows the match, and Escape puts the view back") {
    Kite k(6, 30);  // 4 rows of text
    std::string text;
    for (int i = 0; i < 20; ++i) {
        text += "line " + std::to_string(i) + "\n";
    }
    k.open("lines.txt", text);
    k.press(keys::ctrl_f).type("line 15");
    CHECK(k.rows(0, 4) == "line 12\nline 13\nline 14\nline 15\n");
    CHECK(k.cursor() == "row 3, col 0");
    k.press(keys::esc);
    CHECK(k.line(0) == "line 0");
    CHECK(k.cursor() == "row 0, col 0");
}

TEST_CASE("searching changes nothing in the text") {
    Kite k = cats();
    k.press(keys::ctrl_f).type("cat").press(keys::backspace).press(keys::del).press(keys::enter);
    CHECK(k.rows(0, 4) == "the cat\na cat sat\ncat\n~\n");
    CHECK(k.line(8) == "cats.txt - 3 lines                                       1/3");
}

TEST_CASE("the cursor lands on a match after a tab, on screen") {
    Kite k(10, 60);
    k.open("tab.txt", "\tcat\n");
    k.press(keys::ctrl_f).type("cat");
    CHECK(k.cursor() == "row 0, col 8");
}

TEST_CASE("after Enter, typing edits at the match") {
    Kite k = cats();
    k.press(keys::ctrl_f).type("sat").press(keys::enter).type("x");
    CHECK(k.line(1) == "a cat xsat");
}

}  // namespace ktest
