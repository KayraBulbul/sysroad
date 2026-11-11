#include "doctest.h"

#include <cstdio>
#include <string>
#include <unistd.h>

#include "reference.hpp"

static void check_same(const Counts& got, const Counts& want) {
    CHECK(got.lines == want.lines);
    CHECK(got.words == want.words);
    CHECK(got.bytes == want.bytes);
}

TEST_CASE("count_chunk for every length up to 300") {
    for (std::size_t n = 0; n <= 300; ++n) {
        std::string s = mixed_text(n, static_cast<unsigned>(n));
        CountState st;
        count_chunk(st, s.data(), s.size());
        CAPTURE(n);
        check_same(st.counts, plain_counts(s));
    }
}

TEST_CASE("count_chunk in pieces of every size") {
    std::string s = mixed_text(5000, 3);
    Counts want = plain_counts(s);
    for (std::size_t piece : {1, 2, 15, 16, 17, 31, 100, 4096, 4097}) {
        CountState st;
        for (std::size_t i = 0; i < s.size(); i += piece) {
            count_chunk(st, s.data() + i, std::min(piece, s.size() - i));
        }
        CAPTURE(piece);
        check_same(st.counts, want);
    }
}

TEST_CASE("long runs overflow nothing") {
    for (std::string s : {std::string(70'000, '\n'), std::string(70'000, 'x'), std::string(70'000, ' ')}) {
        std::string alternating;
        CountState st;
        count_chunk(st, s.data(), s.size());
        check_same(st.counts, plain_counts(s));
    }
    std::string pairs;
    for (int i = 0; i < 40'000; ++i) {
        pairs += "a ";
    }
    CountState st;
    count_chunk(st, pairs.data(), pairs.size());
    check_same(st.counts, plain_counts(pairs));
}

TEST_CASE("every whitespace byte separates words, and nothing else does") {
    for (int c = 0; c < 256; ++c) {
        std::string s = "a";
        s += static_cast<char>(c);
        s += "b";
        s = s + s + s + s + s + s;  // long enough for 16-byte blocks
        CountState st;
        count_chunk(st, s.data(), s.size());
        CAPTURE(c);
        check_same(st.counts, plain_counts(s));
    }
}

TEST_CASE("count_fd reads a whole file") {
    std::string s = mixed_text(1'000'003, 11);
    std::FILE* f = std::tmpfile();
    REQUIRE(f != nullptr);
    REQUIRE(std::fwrite(s.data(), 1, s.size(), f) == s.size());
    std::fflush(f);
    REQUIRE(lseek(fileno(f), 0, SEEK_SET) == 0);
    Counts got;
    CHECK(count_fd(fileno(f), got));
    check_same(got, plain_counts(s));
    std::fclose(f);
}

TEST_CASE("count_fd reports a read error") {
    Counts got;
    CHECK_FALSE(count_fd(-1, got));
}
