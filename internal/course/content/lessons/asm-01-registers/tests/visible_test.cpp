#include "doctest.h"

#include "mystery.hpp"

// One example each. Submit checks many more inputs.

TEST_CASE("first") {
    CHECK(first(30, 12) == 42);
}

TEST_CASE("second") {
    CHECK(second(30, 12) == 6);
}

TEST_CASE("third") {
    CHECK(third(1) == 12);
}

TEST_CASE("fourth") {
    CHECK(fourth(9, 4, 10) == 50);
}

TEST_CASE("fifth") {
    CHECK(fifth(10, 30) == 100);
}

TEST_CASE("sixth") {
    CHECK(sixth(1, 8) == 18);
}
