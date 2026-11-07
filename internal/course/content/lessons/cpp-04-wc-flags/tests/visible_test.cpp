#include "doctest.h"

#include "wc.hpp"

TEST_CASE("parse_options: no flags means all three") {
    Options o = parse_options({});
    CHECK(o.lines);
    CHECK(o.words);
    CHECK(o.bytes);
    CHECK(o.bad == 0);
}

TEST_CASE("parse_options: -l") {
    Options o = parse_options({"-l"});
    CHECK(o.lines);
    CHECK_FALSE(o.words);
    CHECK_FALSE(o.bytes);
}

TEST_CASE("format_counts: all three columns") {
    Options all{.lines = true, .words = true, .bytes = true};
    CHECK(format_counts({1, 2, 12}, all) == "      1       2      12\n");
}
