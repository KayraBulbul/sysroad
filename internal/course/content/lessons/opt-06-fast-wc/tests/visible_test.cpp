#include "doctest.h"

#include <cstdio>
#include <string>
#include <unistd.h>

#include "reference.hpp"

static Counts counts_of(const std::string& s) {
    CountState st;
    count_chunk(st, s.data(), s.size());
    return st.counts;
}

TEST_CASE("count_chunk on a short line") {
    Counts c = counts_of("one two  three\n");
    CHECK(c.lines == 1);
    CHECK(c.words == 3);
    CHECK(c.bytes == 15);
}

TEST_CASE("count_chunk on 1000 bytes") {
    std::string s = mixed_text(1000, 1);
    Counts want = plain_counts(s), got = counts_of(s);
    CHECK(got.lines == want.lines);
    CHECK(got.words == want.words);
    CHECK(got.bytes == want.bytes);
}

TEST_CASE("a word split across two chunks counts once") {
    CountState st;
    count_chunk(st, "hel", 3);
    count_chunk(st, "lo world", 8);
    CHECK(st.counts.words == 2);
}

TEST_CASE("count_chunk is at least 3 times as fast as a byte at a time") {
    std::string s = mixed_text(4 << 20, 7);
    Counts want;
    double plain = seconds([&] { want = plain_counts(s); });
    CountState st;
    double yours = seconds([&] { count_chunk(st, s.data(), s.size()); });
    CHECK(st.counts.words == want.words);
    CAPTURE(plain);
    CHECK(yours * 3 < plain);
}
