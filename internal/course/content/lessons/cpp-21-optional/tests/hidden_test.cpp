#include "doctest.h"

#include <limits>

#include "parse.hpp"

TEST_CASE("parse_int: rejects") {
    CHECK(parse_int("") == std::nullopt);
    CHECK(parse_int("abc") == std::nullopt);
    CHECK(parse_int(" 1") == std::nullopt);
    CHECK(parse_int("1 ") == std::nullopt);
    CHECK(parse_int("+1") == std::nullopt);
    CHECK(parse_int("-") == std::nullopt);
}

TEST_CASE("parse_int: limits") {
    CHECK(parse_int("2147483647") == std::numeric_limits<int>::max());
    CHECK(parse_int("-2147483648") == std::numeric_limits<int>::min());
    CHECK(parse_int("2147483648") == std::nullopt);
}

TEST_CASE("parse_size: lower case and G") {
    CHECK(parse_size("1k") == 1024u);
    CHECK(parse_size("3g") == 3ull * 1024 * 1024 * 1024);
    CHECK(parse_size("0M") == 0u);
}

TEST_CASE("parse_size: rejects") {
    CHECK(parse_size("") == std::nullopt);
    CHECK(parse_size("K") == std::nullopt);
    CHECK(parse_size("1.5M") == std::nullopt);
    CHECK(parse_size("4Q") == std::nullopt);
    CHECK(parse_size("-1") == std::nullopt);
    CHECK(parse_size("4KK") == std::nullopt);
}

TEST_CASE("parse_size: overflow") {
    CHECK(parse_size("18446744073709551615") == std::numeric_limits<std::uint64_t>::max());
    CHECK(parse_size("18446744073709551616") == std::nullopt);
    CHECK(parse_size("17179869184G") == std::nullopt);  // 2^34 GiB = 2^64 bytes
    CHECK(parse_size("17179869183G") == 17179869183ull << 30);
}
