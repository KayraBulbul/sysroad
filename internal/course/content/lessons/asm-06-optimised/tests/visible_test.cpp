#include "doctest.h"

#include "mystery.hpp"

// One or two examples each. Submit checks many more inputs.

TEST_CASE("quebec") {
    CHECK(quebec(1234) == 123);
}

TEST_CASE("romeo") {
    CHECK(romeo(4) == 60);
}

TEST_CASE("sierra") {
    CHECK(sierra(22) == 5);
    CHECK(sierra(-22) == -5);
}

TEST_CASE("tango") {
    CHECK(tango('7'));
    CHECK_FALSE(tango('x'));
}

TEST_CASE("uniform") {
    CHECK(uniform(3) == 59);
    CHECK(uniform(13) == -1);
}
