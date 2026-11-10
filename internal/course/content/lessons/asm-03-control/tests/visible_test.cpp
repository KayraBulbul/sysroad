#include "doctest.h"

#include "mystery.hpp"

// One or two examples each. Submit checks many more inputs.

TEST_CASE("alpha") {
    CHECK(alpha(3, 8) == 8);
    CHECK(alpha(-2, -9) == -2);
}

TEST_CASE("bravo") {
    CHECK(bravo(95) == 4);
    CHECK(bravo(10) == 0);
}

TEST_CASE("charlie") {
    CHECK(charlie(2, 10) == 1024);
}

TEST_CASE("delta") {
    CHECK(delta(1) == 0);
    CHECK(delta(6) == 8);
}

TEST_CASE("echo") {
    CHECK(echo(12, 18) == 6);
}
