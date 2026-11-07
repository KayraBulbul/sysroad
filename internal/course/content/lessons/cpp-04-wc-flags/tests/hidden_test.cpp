#include "doctest.h"

#include "wc.hpp"

TEST_CASE("parse_options: bundled flags") {
    Options o = parse_options({"-wc"});
    CHECK_FALSE(o.lines);
    CHECK(o.words);
    CHECK(o.bytes);
}

TEST_CASE("parse_options: separate flags, any order") {
    Options o = parse_options({"-c", "-l"});
    CHECK(o.lines);
    CHECK_FALSE(o.words);
    CHECK(o.bytes);
}

TEST_CASE("parse_options: an unknown letter") {
    CHECK(parse_options({"-lx"}).bad == 'x');
    CHECK(parse_options({"-q", "-l"}).bad == 'q');
}

TEST_CASE("parse_options: non-flags are ignored") {
    Options o = parse_options({"file.txt", "-", "-w"});
    CHECK_FALSE(o.lines);
    CHECK(o.words);
    CHECK(o.bad == 0);
}

TEST_CASE("format_counts: columns keep their order") {
    Options o{.lines = true, .bytes = true};
    CHECK(format_counts({3, 9, 44}, o) == "      3      44\n");
    Options w{.words = true};
    CHECK(format_counts({3, 9, 44}, w) == "      9\n");
}

TEST_CASE("format_counts: a number wider than the column") {
    Options b{.bytes = true};
    CHECK(format_counts({0, 0, 123456789}, b) == "123456789\n");
}
