#include "doctest.h"

#include <string>

#include "simd.hpp"

static std::size_t plain_count(const char* s, std::size_t n, char c) {
    std::size_t k = 0;
    for (std::size_t i = 0; i < n; ++i) {
        k += s[i] == c;
    }
    return k;
}

static std::string mixed(std::size_t n) {
    std::string s(n, ' ');
    for (std::size_t i = 0; i < n; ++i) {
        s[i] = static_cast<char>("ab\nc \xff\x80"[i * 7 % 8]);
    }
    return s;
}

TEST_CASE("count_byte for every length and starting offset") {
    std::string s = mixed(200);
    for (std::size_t start = 0; start < 17; ++start) {
        for (std::size_t n = 0; start + n <= 120; ++n) {
            CAPTURE(start);
            CAPTURE(n);
            CHECK(count_byte(s.data() + start, n, '\n') == plain_count(s.data() + start, n, '\n'));
        }
    }
}

TEST_CASE("count_byte when every byte matches, past 255 blocks") {
    // 255 blocks of 16 bytes is 4080 bytes. More than that, all matching,
    // overflows a lane counter that's never emptied.
    for (std::size_t n : {4080, 4096, 10'000, 65'537}) {
        std::string s(n, 'x');
        CAPTURE(n);
        CHECK(count_byte(s.data(), n, 'x') == n);
        CHECK(count_byte(s.data(), n, 'y') == 0);
    }
}

TEST_CASE("count_byte of bytes above 127") {
    std::string s = mixed(1000);
    CHECK(count_byte(s.data(), s.size(), '\xff') == plain_count(s.data(), s.size(), '\xff'));
    CHECK(count_byte(s.data(), s.size(), '\x80') == plain_count(s.data(), s.size(), '\x80'));
    CHECK(count_byte(s.data(), s.size(), '\0') == plain_count(s.data(), s.size(), '\0'));
}

TEST_CASE("is_ascii looks at every byte") {
    std::string s(1000, 'a');
    CHECK(is_ascii(s.data(), s.size()));
    CHECK(is_ascii(s.data(), 0));
    for (std::size_t pos : {0, 1, 15, 16, 500, 998, 999}) {
        std::string t = s;
        t[pos] = '\x80';
        CAPTURE(pos);
        CHECK_FALSE(is_ascii(t.data(), t.size()));
        CHECK(is_ascii(t.data(), pos));
    }
    std::string del(5, '\x7f');
    CHECK(is_ascii(del.data(), del.size()));
}
