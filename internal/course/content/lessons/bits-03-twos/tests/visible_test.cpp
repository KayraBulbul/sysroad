#include "doctest.h"

#include <cstdint>

#include "twos.hpp"

TEST_CASE("sign_extend reads the low bits as signed") {
    CHECK(sign_extend(200, 8) == -56);
    CHECK(sign_extend(200, 9) == 200);
    CHECK(sign_extend(0xff, 8) == -1);
    CHECK(sign_extend(0x7f, 8) == 127);
    CHECK(sign_extend(0x80, 8) == -128);
}

TEST_CASE("add_overflows") {
    CHECK_FALSE(add_overflows(2, 3));
    CHECK(add_overflows(INT32_MAX, 1));
    CHECK(add_overflows(INT32_MIN, -1));
    CHECK_FALSE(add_overflows(INT32_MAX, -1));
}

TEST_CASE("wrapping_add wraps around") {
    CHECK(wrapping_add(2, 3) == 5);
    CHECK(wrapping_add(INT32_MAX, 1) == INT32_MIN);
    CHECK(wrapping_add(INT32_MIN, -1) == INT32_MAX);
}

TEST_CASE("saturating_add stops at the limits") {
    CHECK(saturating_add(2, 3) == 5);
    CHECK(saturating_add(INT32_MAX, 1) == INT32_MAX);
    CHECK(saturating_add(INT32_MIN, -1) == INT32_MIN);
}

TEST_CASE("checked_mul") {
    CHECK(checked_mul(6, 7) == 42);
    CHECK(checked_mul(65536, 65536) == std::nullopt);
}
