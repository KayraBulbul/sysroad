#include "doctest.h"

#include <sstream>

#include "wc.hpp"

static Counts count(const std::string& s) {
    std::istringstream in(s);
    return count_stream(in);
}

TEST_CASE("count_stream: one line") {
    Counts c = count("hello world\n");
    CHECK(c.lines == 1);
    CHECK(c.words == 2);
    CHECK(c.bytes == 12);
}

TEST_CASE("count_stream: empty input") {
    Counts c = count("");
    CHECK(c.lines == 0);
    CHECK(c.words == 0);
    CHECK(c.bytes == 0);
}
