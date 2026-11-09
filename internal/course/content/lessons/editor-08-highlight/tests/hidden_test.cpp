#include "doctest.h"

#include "codes.hpp"

TEST_CASE("hl has one entry per render character, tabs included") {
    Row r = row("\tint x;");
    highlight_row(r, false);
    CHECK(r.hl.size() == r.render.size());
    CHECK(codes(r) == "nnnnnnnntttnnn");
}

TEST_CASE("keywords only as whole words") {
    CHECK(hl("format(x)") == "nnnnnnnnn");  // "for" inside a word
    CHECK(hl("if(intx)") == "kknnnnnn");    // "int" with no separator after
    CHECK(hl("x_int") == "nnnnn");          // nor before
    CHECK(hl("else{") == "kkkkn");
    CHECK(hl("auto n=size_t(0);") == "ttttnnnttttttndnn");
}

TEST_CASE("numbers: decimals, suffixes, hex, and not inside names") {
    CHECK(hl("3.14f") == "ddddd");
    CHECK(hl("0xff") == "dddd");
    CHECK(hl("x1 = 10u") == "nnnnnddd");
    CHECK(hl("a[2]") == "nndn");
    CHECK(hl("-1") == "nd");
}

TEST_CASE("strings: both quotes, escapes, and an unclosed string runs on") {
    CHECK(hl("'a'") == "sss");
    CHECK(hl("\"a\\\"b\" x") == "ssssssnn");  // "a\"b" x
    CHECK(hl("\"// not a comment\"") == "ssssssssssssssssss");
    CHECK(hl("\"open") == "sssss");
    CHECK(hl("'\\\\' 1") == "ssssnd");  // '\\' 1
}

TEST_CASE("block comments open, close, and carry over") {
    CHECK(hl("a /* b */ c") == "nnmmmmmmmnn");
    CHECK(hl("int /* open") == "tttnmmmmmmm");
    Row r = row("int /* open");
    CHECK(highlight_row(r, false));
    Row mid = row("still inside");
    CHECK(highlight_row(mid, true));
    CHECK(codes(mid) == "mmmmmmmmmmmm");
    Row end = row("end */ 1");
    CHECK_FALSE(highlight_row(end, true));
    CHECK(codes(end) == "mmmmmmnd");
    Row closed = row("/* x */");
    CHECK_FALSE(highlight_row(closed, false));
}

TEST_CASE("comment markers inside strings and comments don't count") {
    CHECK(hl("\"/*\" x") == "ssssnn");
    CHECK(hl("// a /* b") == "ccccccccc");
    Row r = row("// a /* b");
    CHECK_FALSE(highlight_row(r, false));
    CHECK(hl("/* \"x */ 2") == "mmmmmmmmnd");
}

TEST_CASE("preprocessor lines") {
    CHECK(hl("#include <vector>") == "ppppppppppppppppp");
    CHECK(hl("#include \"kite.hpp\"") == "pppppppppssssssssss");
    CHECK(hl("  #define N 10 // ten") == "nnppppppppppddpcccccc");
    CHECK(hl("x # y") == "nnnnn");
}

TEST_CASE("an empty line, and a line that's only a comment's end") {
    Row r = row("");
    CHECK_FALSE(highlight_row(r, false));
    CHECK(r.hl.empty());
    Row e = row("");
    CHECK(highlight_row(e, true));
    CHECK(hl("*/", true) == "mm");
}

TEST_CASE("update_syntax carries a comment down the file, and stops when it can") {
    Editor e;
    e.highlight = true;
    for (const char* line : {"a", "b", "c", "d"}) {
        e.rows.push_back(row(line));
    }
    update_syntax(e, 0);
    e.rows[1].chars = "/* b";
    refresh_row(e, 1);
    CHECK(codes(e.rows[2]) == "m");
    CHECK(codes(e.rows[3]) == "m");
    e.rows[2].chars = "c */";
    refresh_row(e, 2);
    CHECK(codes(e.rows[3]) == "n");
}
