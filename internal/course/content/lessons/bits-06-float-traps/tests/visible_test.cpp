#include "doctest.h"

#include <cmath>
#include <format>
#include <limits>
#include <string>

#include "traps.hpp"

// The shortest text that reads back as the same double, so a failure shows
// digits that doctest would round away.
static std::string repr(double x) {
    return std::format("{}", x);
}

static const double kNaN = std::numeric_limits<double>::quiet_NaN();

TEST_CASE("close_enough forgives rounding") {
    CHECK(close_enough(0.1 + 0.2, 0.3));
    CHECK(close_enough(1.0, 1.0));
    CHECK_FALSE(close_enough(1.0, 1.001));
}

TEST_CASE("elapsed_seconds doesn't drift") {
    CHECK(repr(elapsed_seconds(10)) == "1");
    CHECK(repr(elapsed_seconds(36000)) == "3600");
}

TEST_CASE("largest skips NaNs") {
    CHECK(largest({1, 5, 2}) == 5);
    CHECK(largest({kNaN, 1, 2}) == 2);
    CHECK(largest({1, kNaN, 2}) == 2);
}

TEST_CASE("to_cents rounds to the nearest cent") {
    CHECK(to_cents(1.5) == 150);
    CHECK(to_cents(0.29) == 29);
}

TEST_CASE("parse_id keeps every digit") {
    CHECK(parse_id("42") == 42u);
    CHECK(parse_id("9007199254740993") == 9007199254740993u);
    CHECK(parse_id("12x") == std::nullopt);
}
