#include "doctest.h"

#include <string>

#include "simd.hpp"

TEST_CASE("is_ascii") {
    std::string plain = "hello, world\n";
    std::string accented = "caf\xc3\xa9";
    CHECK(is_ascii(plain.data(), plain.size()));
    CHECK_FALSE(is_ascii(accented.data(), accented.size()));
}

TEST_CASE("count_byte on less than 16 bytes") {
    std::string s = "a\nb\nc\n";
    CHECK(count_byte(s.data(), s.size(), '\n') == 3);
}

TEST_CASE("count_byte on more than 16 bytes") {
    std::string s = "one\ntwo\nthree\nfour\nfive\nsix\nseven\n";  // 34 bytes
    CHECK(count_byte(s.data(), s.size(), '\n') == 7);
    CHECK(count_byte(s.data(), s.size(), 'e') == 6);
}
