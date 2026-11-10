#include "doctest.h"

#include "bits.hpp"

TEST_CASE("to_binary groups the 32 bits in fours") {
    CHECK(to_binary(202) == "0000 0000 0000 0000 0000 0000 1100 1010");
    CHECK(to_binary(0) == "0000 0000 0000 0000 0000 0000 0000 0000");
    CHECK(to_binary(0x80000001) == "1000 0000 0000 0000 0000 0000 0000 0001");
}

TEST_CASE("parse_number reads decimal, hex and binary") {
    CHECK(parse_number("202") == 202u);
    CHECK(parse_number("0xca") == 202u);
    CHECK(parse_number("0b11001010") == 202u);
    CHECK(parse_number("0") == 0u);
}

TEST_CASE("parse_number rejects what isn't a number") {
    CHECK(parse_number("") == std::nullopt);
    CHECK(parse_number("12a") == std::nullopt);
    CHECK(parse_number("0b102") == std::nullopt);
}

TEST_CASE("count_ones") {
    CHECK(count_ones(0) == 0);
    CHECK(count_ones(202) == 4);
    CHECK(count_ones(0xffffffff) == 32);
}

TEST_CASE("is_power_of_two") {
    CHECK(is_power_of_two(1));
    CHECK(is_power_of_two(64));
    CHECK_FALSE(is_power_of_two(202));
    CHECK_FALSE(is_power_of_two(0));
}
