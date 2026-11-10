#include "doctest.h"

#include <climits>
#include <cstdint>
#include <string>
#include <vector>

#include "conv.hpp"

TEST_CASE("below at the edges") {
    CHECK(below(INT_MIN, 0));
    CHECK(below(-1, 0));
    CHECK_FALSE(below(0, 0));
    CHECK(below(0, 1));
    CHECK(below(INT_MAX, UINT_MAX));
    CHECK(below(INT_MAX, static_cast<unsigned>(INT_MAX) + 1));
    CHECK_FALSE(below(INT_MAX, static_cast<unsigned>(INT_MAX)));
    CHECK(below(-5, 4294967291u));  // the value -5 turns into
    CHECK(below(7, 4000000000u));
}

TEST_CASE("last_index_of on small and empty vectors") {
    CHECK(last_index_of({}, 1) == -1);
    CHECK(last_index_of({1}, 1) == 0);
    CHECK(last_index_of({1}, 2) == -1);
    CHECK(last_index_of({5, 5, 5}, 5) == 2);
    CHECK(last_index_of({5, 6, 7}, 5) == 0);
    std::vector<int> big(10000, 3);
    big[1234] = 8;
    CHECK(last_index_of(big, 8) == 1234);
    CHECK(last_index_of(big, 3) == 9999);
    CHECK(last_index_of(big, 4) == -1);
}

TEST_CASE("byte_histogram over all 256 values") {
    std::string s;
    for (int v = 0; v < 256; ++v) {
        s.append(static_cast<std::size_t>(v % 3 + 1), static_cast<char>(v));
    }
    auto counts = byte_histogram(s);
    for (int v = 0; v < 256; ++v) {
        CAPTURE(v);
        REQUIRE(counts[static_cast<std::size_t>(v)] == v % 3 + 1);
    }
    auto none = byte_histogram("");
    for (int c : none) {
        REQUIRE(c == 0);
    }
}

TEST_CASE("average of every pair of bytes") {
    for (int a = 0; a < 256; ++a) {
        for (int b = 0; b < 256; ++b) {
            if (average(static_cast<std::uint8_t>(a), static_cast<std::uint8_t>(b)) !=
                (a + b) / 2) {
                CAPTURE(a);
                CAPTURE(b);
                FAIL("wrong average");
            }
        }
    }
}

TEST_CASE("mib_to_bytes up to the largest input") {
    CHECK(mib_to_bytes(0) == 0);
    CHECK(mib_to_bytes(4095) == 4293918720u);
    CHECK(mib_to_bytes(4096) == 4294967296u);
    CHECK(mib_to_bytes(4097) == 4296015872u);
    CHECK(mib_to_bytes(0xffffffff) == 4503599626321920u);
}

TEST_CASE("bit sets exactly one bit, at every position") {
    for (int n = 0; n < 64; ++n) {
        std::uint64_t expected = 1;
        for (int i = 0; i < n; ++i) {
            expected *= 2;
        }
        CAPTURE(n);
        REQUIRE(bit(n) == expected);
    }
}
