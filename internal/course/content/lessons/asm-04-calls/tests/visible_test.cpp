#include "doctest.h"

#include "mystery.hpp"

// One example each. Submit checks many more inputs.

TEST_CASE("foxtrot") {
    CHECK(foxtrot(1, 2, 3, 4, 5, 6, 7, 8) == 64);
}

TEST_CASE("golf") {
    CHECK(golf(5) == 26);
}

TEST_CASE("hotel") {
    CHECK(hotel(3, 4) == 25);
}

TEST_CASE("india") {
    CHECK(india(0) == 0);
    CHECK(india(3) == 12);
}

TEST_CASE("juliet") {
    CHECK(juliet(10) == 50);
}
