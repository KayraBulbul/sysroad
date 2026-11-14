#include "doctest.h"

#include <string>

#include "colours.hpp"

namespace ktest {

TEST_CASE("keywords and types only as whole words") {
    CHECK(hl("format(x)") == "nnnnnnnnn");  // "for" inside a word
    CHECK(hl("if(intx)") == "kknnnnnn");    // "int" with no separator after
    CHECK(hl("x_int") == "nnnnn");          // nor before
    CHECK(hl("else{") == "kkkkn");
    CHECK(hl("auto n=size_t(0);") == "ttttnnnttttttndnn");
    CHECK(hl("constexpr unsigned") == "kkkkkkkkkntttttttt");
}

TEST_CASE("numbers and strings still follow last lesson's rules") {
    CHECK(hl("3.14f + 0xff") == "dddddnnndddd");
    CHECK(hl("\"a\\\"b\" x") == "ssssssnn");
    CHECK(hl("'\\\\' 1") == "ssssnd");
}

TEST_CASE("comment markers inside strings, and inside comments, don't count") {
    CHECK(hl("\"/*\" x") == "ssssnn");
    CHECK(hl("\"// not a comment\"") == "ssssssssssssssssss");
    CHECK(hl("/* \"x */ 2") == "ccccccccnd");
    Kite k(8, 40);
    k.open("markers.cpp", "// a /* b\nx\n");
    CHECK(colours(k, 0, 9) == "ccccccccc");
    CHECK_MESSAGE(colours(k, 1, 1) == "n", "a /* inside a // comment doesn't open a block comment");
}

TEST_CASE("a block comment can open and close on one line, more than once") {
    CHECK(hl("a /* b */ c") == "nncccccccnn");
    CHECK(hl("/**/1/* */2") == "ccccdcccccd");
}

TEST_CASE("preprocessor lines are blue, apart from their strings, numbers and comments") {
    CHECK(hl("#include <vector>") == "ppppppppppppppppp");
    CHECK(hl("#include \"kite.hpp\"") == "pppppppppssssssssss");
    CHECK(hl("  #define N 10 // ten") == "nnppppppppppddpcccccc");
    CHECK(hl("x # y") == "nnnnn");
    CHECK(hl("#if int") == "ppppppp");
}

TEST_CASE("an empty line inside a comment, and a line that's only its end") {
    Kite k(8, 40);
    k.open("empty.cpp", "/*\n\n*/ 1\nx\n");
    CHECK(colours(k, 0, 2) == "cc");
    CHECK(colours(k, 2, 4) == "ccnd");
    CHECK(colours(k, 3, 1) == "n");
}

TEST_CASE("typing /* colours the lines below, and typing */ ends it") {
    Kite k(8, 40);
    k.open("edit.cpp", "a\nb\nc\nd\n");
    k.press(keys::down).press(keys::home).type("/*");
    CHECK(colours(k, 1, 3) == "ccc");
    CHECK(colours(k, 2, 1) == "c");
    CHECK(colours(k, 3, 1) == "c");
    k.press(keys::down).press(keys::end).type("*/");
    CHECK(colours(k, 2, 3) == "ccc");
    CHECK(colours(k, 3, 1) == "n");
}

TEST_CASE("deleting the /* puts the lines below back") {
    Kite k(8, 40);
    k.open("undo.cpp", "/* a\nb\nc */ 1\nd\n");
    CHECK(colours(k, 1, 1) == "c");
    k.press(keys::del, 2);
    CHECK(colours(k, 0, 2) == "nn");
    CHECK(colours(k, 1, 1) == "n");
    CHECK(colours(k, 2, 6) == "nnnnnd");
}

TEST_CASE("a comment opened at the top reaches rows far below the screen") {
    std::string text;
    for (int i = 0; i < 30; ++i) {
        text += "x" + std::to_string(i) + "\n";
    }
    Kite k(6, 40);  // 4 rows of text
    k.open("long.cpp", text);
    k.type("/*").press(keys::page_down, 8);
    CHECK(k.line(2) == "x29");
    CHECK(colours(k, 2, 3) == "ccc");
}

TEST_CASE("Enter and joins keep comments right") {
    Kite k(8, 40);
    k.open("split.cpp", "a /* b */ c\nd\n");
    k.press(keys::right, 5).press(keys::enter);  // splits inside the comment
    CHECK(colours(k, 0, 4) == "nncc");
    CHECK(colours(k, 1, 6) == "ccccnn");
    CHECK(colours(k, 2, 1) == "n");
    k.press(keys::backspace);  // and joins it again
    CHECK(colours(k, 0, 11) == "nncccccccnn");
}

TEST_CASE("a file that isn't C++ still has no colours") {
    CHECK(hl("int /* x */ 1", "plain.txt") == "nnnnnnnnnnnnn");
}

}  // namespace ktest
