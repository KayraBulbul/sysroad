#include "doctest.h"

#include <sstream>
#include <string>

#include "wc.hpp"

static Counts count(const std::string& s) {
    std::istringstream in(s);
    return count_stream(in);
}

TEST_CASE("count_stream: no newline at the end") {
    Counts c = count("no newline");
    CHECK(c.lines == 0);
    CHECK(c.words == 2);
    CHECK(c.bytes == 10);
}

TEST_CASE("count_stream: blank lines") {
    Counts c = count("\n\n\n");
    CHECK(c.lines == 3);
    CHECK(c.words == 0);
    CHECK(c.bytes == 3);
}

TEST_CASE("count_stream: a word split across two 4096-byte reads") {
    std::string s(4095, 'x');  // one long word ...
    s += "yz more\n";          // ... that continues past byte 4096
    Counts c = count(s);
    CHECK(c.words == 2);
    CHECK(c.lines == 1);
    CHECK(c.bytes == s.size());
}

TEST_CASE("count_stream: whitespace exactly at the chunk boundary") {
    std::string s(4096, 'a');
    s += " b";
    Counts c = count(s);
    CHECK(c.words == 2);
}

TEST_CASE("count_stream: a lot of input") {
    std::string s;
    for (int i = 0; i < 5000; ++i) {
        s += "one two three\n";
    }
    Counts c = count(s);
    CHECK(c.lines == 5000);
    CHECK(c.words == 15000);
    CHECK(c.bytes == s.size());
}
