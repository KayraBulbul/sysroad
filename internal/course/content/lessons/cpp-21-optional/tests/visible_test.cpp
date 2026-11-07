#include "doctest.h"

#include "parse.hpp"

TEST_CASE("parse_int") {
    CHECK(parse_int("42") == 42);
    CHECK(parse_int("-7") == -7);
    CHECK(parse_int("12abc") == std::nullopt);
}

TEST_CASE("parse_size: suffixes") {
    CHECK(parse_size("12") == 12u);
    CHECK(parse_size("4K") == 4096u);
    CHECK(parse_size("2M") == 2u * 1024 * 1024);
}
