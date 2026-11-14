#include "doctest.h"

#include "colours.hpp"

namespace ktest {

TEST_CASE("numbers: decimals, suffixes, hex, and not inside names") {
    CHECK(hl("3.14f") == "ddddd");
    CHECK(hl("0xff") == "dddd");
    CHECK(hl("x1 = 10u") == "nnnnnddd");
    CHECK(hl("a[2]") == "nndn");
    CHECK(hl("-1") == "nd");
    CHECK(hl("f(3,4)") == "nndndn");
    CHECK(hl("x_2") == "nnn");
}

TEST_CASE("strings: both quotes, escapes, and an unclosed string runs on") {
    CHECK(hl("'a'") == "sss");
    CHECK(hl("\"a\\\"b\" x") == "ssssssnn");  // "a\"b" x
    CHECK(hl("\"open") == "sssss");
    CHECK(hl("'\\\\' 1") == "ssssnd");  // '\\' 1
    CHECK(hl("\"it's\"") == "ssssss");
    CHECK(hl("'\"' 2") == "sssnd");
}

TEST_CASE("after a string, a number can start") {
    CHECK(hl("\"a\"1") == "sssd");
}

TEST_CASE("tabs: the colours line up with the screen") {
    CHECK(hl("\tx = 1;") == "nnnnnnnnnnnndn");
}

TEST_CASE("every C and C++ file name gets colours, and nothing else does") {
    for (std::string name : {"a.c", "a.h", "a.cc", "a.cpp", "a.cxx", "a.hh", "a.hpp", "a.hxx"}) {
        INFO(("the file " + name));
        CHECK(hl("1", name) == "d");
    }
    for (std::string name : {"a.txt", "Makefile", "a.cpp.bak", "cpp", "a.py"}) {
        INFO(("the file " + name));
        CHECK(hl("1", name) == "n");
    }
}

TEST_CASE("the colours follow your edits") {
    Kite k(6, 40);
    k.open("edit.cpp", "x = \n");
    k.press(keys::end).type("7");
    CHECK(colours(k, 0, 5) == "nnnnd");
    k.press(keys::home).type("\"");
    CHECK(colours(k, 0, 6) == "ssssss");
    k.press(keys::backspace);
    CHECK(colours(k, 0, 5) == "nnnnd");
}

TEST_CASE("Enter in the middle of a string splits its colours too") {
    Kite k(6, 40);
    k.open("split.cpp", "x = \"ab\" 5\n");
    k.press(keys::right, 6).press(keys::enter);
    CHECK(k.line(0) == "x = \"a");
    CHECK(colours(k, 0, 6) == "nnnnss");
    CHECK(k.line(1) == "b\" 5");
    CHECK(colours(k, 1, 4) == "nsss");
}

TEST_CASE("joining lines colours the joined line") {
    Kite k(6, 40);
    k.open("join.cpp", "x\n1\n");
    k.press(keys::end).press(keys::del);
    CHECK(colours(k, 0, 2) == "nn");
    k.type(" ");
    CHECK(colours(k, 0, 3) == "nnd");
}

TEST_CASE("saving under a C++ name turns colours on") {
    Kite k(6, 40);
    k.draw().type("1").press(keys::ctrl_s).type("new.cpp").press(keys::enter);
    CHECK(colours(k, 0, 1) == "d");
}

TEST_CASE("a control character inside a string keeps the string's colour around it") {
    Kite k(6, 40);
    k.open("ctrl.cpp", "\"a\x01" "b\"\n");
    CHECK(k.line(0) == "\"aAb\"");
    CHECK(colours(k, 0, 5) == "ss#ss");
}

TEST_CASE("DEL shows as an inverted ?, and \\r in the middle of a line as M") {
    Kite k(6, 40);
    k.open("del.txt", "a\x7f" "b\rc\n");
    CHECK(k.line(0) == "a?bMc");
    CHECK(colours(k, 0, 5) == "n#n#n");
}

TEST_CASE("the match follows the arrows, and goes when the search ends") {
    Kite k(6, 40);
    k.open("cats.txt", "the cat\na cat sat\n");
    k.press(keys::ctrl_f).type("cat").press(keys::right);
    CHECK(colours(k, 0, 7) == "nnnnnnn");
    CHECK(colours(k, 1, 9) == "nn###nnnn");
    k.press(keys::esc);
    CHECK(colours(k, 1, 9) == "nnnnnnnnn");
}

TEST_CASE("the match is inverted where it is on screen, after a tab") {
    Kite k(6, 40);
    k.open("tab.txt", "\tcat\n");
    k.press(keys::ctrl_f).type("cat");
    CHECK(colours(k, 0, 11) == "nnnnnnnn###");
}

TEST_CASE("a match in a C++ file is inverted over its colours") {
    Kite k(6, 40);
    k.open("m.cpp", "x = 42;\n");
    k.press(keys::ctrl_f).type("2;");
    CHECK(colours(k, 0, 7) == "nnnnd##");
}

TEST_CASE("scrolled sideways, the colours stay with their characters") {
    Kite k(6, 10);
    k.open("long.cpp", "x = 1234567890123\n");
    k.press(keys::end);
    CHECK(k.line(0) == "567890123");
    CHECK(colours(k, 0, 9) == "ddddddddd");
}

}  // namespace ktest
