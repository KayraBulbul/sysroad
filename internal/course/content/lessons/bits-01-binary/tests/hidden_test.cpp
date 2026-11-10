#include "doctest.h"

#include <bit>
#include <bitset>
#include <cstdint>
#include <string>

#include "bits.hpp"

static std::string reference(std::uint32_t x) {
    std::string raw = std::bitset<32>(x).to_string();
    std::string out;
    for (int i = 0; i < 32; ++i) {
        if (i != 0 && i % 4 == 0) {
            out += ' ';
        }
        out += raw[i];
    }
    return out;
}

TEST_CASE("to_binary agrees with std::bitset on many values") {
    std::uint32_t x = 0x9e3779b9;
    for (int i = 0; i < 200; ++i) {
        x = x * 1664525 + 1013904223;
        CAPTURE(x);
        REQUIRE(to_binary(x) == reference(x));
    }
    CHECK(to_binary(0xffffffff) == "1111 1111 1111 1111 1111 1111 1111 1111");
}

TEST_CASE("parse_number: capitals, leading zeros and the largest values") {
    CHECK(parse_number("0XCA") == 202u);
    CHECK(parse_number("0xDeadBeef") == 0xdeadbeefu);
    CHECK(parse_number("0B101") == 5u);
    CHECK(parse_number("007") == 7u);
    CHECK(parse_number("4294967295") == 0xffffffffu);
    CHECK(parse_number("0xffffffff") == 0xffffffffu);
    CHECK(parse_number("0b11111111111111111111111111111111") == 0xffffffffu);
}

TEST_CASE("parse_number: numbers that don't fit in 32 bits") {
    CHECK(parse_number("4294967296") == std::nullopt);
    CHECK(parse_number("0x100000000") == std::nullopt);
    CHECK(parse_number("0b100000000000000000000000000000000") == std::nullopt);
    // Big enough to wrap a 64-bit accumulator too.
    CHECK(parse_number("99999999999999999999999999") == std::nullopt);
    CHECK(parse_number("0xffffffffffffffffffff") == std::nullopt);
}

TEST_CASE("parse_number: malformed input") {
    CHECK(parse_number("0x") == std::nullopt);
    CHECK(parse_number("0b") == std::nullopt);
    CHECK(parse_number("0xg1") == std::nullopt);
    CHECK(parse_number("-5") == std::nullopt);
    CHECK(parse_number("+5") == std::nullopt);
    CHECK(parse_number(" 5") == std::nullopt);
    CHECK(parse_number("5 ") == std::nullopt);
    CHECK(parse_number("ca") == std::nullopt);
    CHECK(parse_number("1'000") == std::nullopt);
    CHECK(parse_number("9:") == std::nullopt);
    CHECK(parse_number("0x@") == std::nullopt);
    CHECK(parse_number("0x`") == std::nullopt);
    CHECK(parse_number("0xG") == std::nullopt);
}

TEST_CASE("count_ones and is_power_of_two agree with <bit>") {
    std::uint32_t x = 12345;
    for (int i = 0; i < 500; ++i) {
        x = x * 1664525 + 1013904223;
        CAPTURE(x);
        REQUIRE(count_ones(x) == std::popcount(x));
        REQUIRE(is_power_of_two(x) == std::has_single_bit(x));
    }
    for (int i = 0; i < 32; ++i) {
        CAPTURE(i);
        CHECK(is_power_of_two(1u << i));
        CHECK(count_ones(1u << i) == 1);
        if (i > 1) {
            CHECK_FALSE(is_power_of_two((1u << i) - 1));
            CHECK_FALSE(is_power_of_two((1u << i) + 1));
        }
    }
}
