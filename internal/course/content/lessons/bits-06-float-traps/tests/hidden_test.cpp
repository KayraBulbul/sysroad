#include "doctest.h"

#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <string>

#include "traps.hpp"

static std::string repr(double x) {
    return std::format("{}", x);
}

static const double kNaN = std::numeric_limits<double>::quiet_NaN();
static const double kInf = std::numeric_limits<double>::infinity();

TEST_CASE("close_enough scales with the numbers") {
    CHECK(close_enough(1e15 + 0.25, 1e15 + 0.5));
    CHECK(close_enough(123456789.0, 123456789.01));
    CHECK_FALSE(close_enough(123456789.0, 123456790.0));
    CHECK(close_enough(1e300, 1e300 * (1 + 1e-12)));
    CHECK_FALSE(close_enough(1e300, 1.1e300));
    CHECK(close_enough(-2.5e6, -2.5e6 - 1e-6));
    CHECK_FALSE(close_enough(-1.0, 1.0));
    CHECK_FALSE(close_enough(1000.0, 1000.001));
}

TEST_CASE("close_enough near zero uses the floor of 1") {
    CHECK(close_enough(0.0, 1e-12));
    CHECK(close_enough(1e-12, 2e-12));
    CHECK(close_enough(-1e-10, 1e-10));
    CHECK_FALSE(close_enough(0.0, 1e-6));
    CHECK_FALSE(close_enough(1e-7, 2e-7));
    CHECK(close_enough(0.0, -0.0));
}

TEST_CASE("close_enough with NaN and infinity") {
    CHECK_FALSE(close_enough(kNaN, kNaN));
    CHECK_FALSE(close_enough(kNaN, 1.0));
    CHECK_FALSE(close_enough(1.0, kNaN));
    CHECK(close_enough(kInf, kInf));
    CHECK(close_enough(-kInf, -kInf));
    CHECK_FALSE(close_enough(kInf, -kInf));
    CHECK_FALSE(close_enough(kInf, 1e308));
    CHECK_FALSE(close_enough(1e308, kInf));
    CHECK_FALSE(close_enough(kInf, kNaN));
}

TEST_CASE("elapsed_seconds is exactly ticks / 10") {
    CHECK(repr(elapsed_seconds(0)) == "0");
    CHECK(repr(elapsed_seconds(1)) == "0.1");
    CHECK(repr(elapsed_seconds(3)) == "0.3");
    CHECK(repr(elapsed_seconds(7)) == "0.7");
    CHECK(repr(elapsed_seconds(12345)) == "1234.5");
    CHECK(repr(elapsed_seconds(1000001)) == "100000.1");
    for (int ticks = 0; ticks < 5000; ++ticks) {
        if (elapsed_seconds(ticks) != ticks / 10.0) {
            CAPTURE(ticks);
            FAIL("not the nearest double to ticks / 10");
        }
    }
}

TEST_CASE("elapsed_seconds after a long time") {
    // 100 hours, as in the Patriot failure; and about 6.8 years.
    CHECK(repr(elapsed_seconds(3600000)) == "360000");
    CHECK(repr(elapsed_seconds(2147483647)) == "214748364.7");
}

TEST_CASE("largest with NaNs anywhere, or nothing else") {
    CHECK(largest({3, kNaN, 1}) == 3);
    CHECK(largest({kNaN, kNaN, -4, kNaN}) == -4);
    CHECK(largest({kNaN, -7, -2, kNaN, -9}) == -2);
    CHECK(largest({-kInf, kNaN}) == -kInf);
    CHECK(largest({kNaN, -kInf}) == -kInf);
    CHECK(largest({1, kInf, kNaN}) == kInf);
    CHECK(std::isnan(largest({})));
    CHECK(std::isnan(largest({kNaN})));
    CHECK(std::isnan(largest({kNaN, kNaN, kNaN})));
    CHECK(largest({-1.5}) == -1.5);
}

TEST_CASE("to_cents on prices that land just below a whole cent") {
    CHECK(to_cents(1.15) == 115);
    CHECK(to_cents(4.35) == 435);
    CHECK(to_cents(8.2) == 820);
    CHECK(to_cents(0.57) == 57);
    CHECK(to_cents(1.1) == 110);
    CHECK(to_cents(0.0) == 0);
    CHECK(to_cents(19.99) == 1999);
    CHECK(to_cents(1234567.89) == 123456789);
    CHECK(to_cents(-0.29) == -29);
    CHECK(to_cents(-4.35) == -435);
    CHECK(to_cents(0.014) == 1);
    CHECK(to_cents(0.016) == 2);
    for (int cents = 0; cents < 20000; ++cents) {
        if (to_cents(cents / 100.0) != cents) {
            CAPTURE(cents);
            FAIL("a price didn't survive the round trip");
        }
    }
}

TEST_CASE("parse_id above 2^53") {
    CHECK(parse_id("9007199254740992") == 9007199254740992u);
    CHECK(parse_id("9007199254740995") == 9007199254740995u);
    CHECK(parse_id("1234567890123456789") == 1234567890123456789u);
    CHECK(parse_id("18446744073709551615") == 18446744073709551615u);
    CHECK(parse_id("18446744073709551614") == 18446744073709551614u);
    CHECK(parse_id("0") == 0u);
    CHECK(parse_id("007") == 7u);
}

TEST_CASE("parse_id rejects what isn't an id") {
    CHECK(parse_id("") == std::nullopt);
    CHECK(parse_id("18446744073709551616") == std::nullopt);
    CHECK(parse_id("99999999999999999999999") == std::nullopt);
    CHECK(parse_id("-1") == std::nullopt);
    CHECK(parse_id("+1") == std::nullopt);
    CHECK(parse_id("1.5") == std::nullopt);
    CHECK(parse_id("1e3") == std::nullopt);
    CHECK(parse_id(" 1") == std::nullopt);
    CHECK(parse_id("1 ") == std::nullopt);
    CHECK(parse_id("0x10") == std::nullopt);
}
