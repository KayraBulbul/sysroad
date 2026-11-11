#include "doctest.h"

#include "report.hpp"
#include "stats.hpp"

TEST_CASE("each file's less_than is its own") {
    verbosity = 1;
    // stats.cpp compares numbers: 2.001 is the largest.
    CHECK(largest({2.001, 2.0}) == 2.001);
    CHECK(smallest({2.001, 2.0}) == 2.0);
    // report.cpp compares at two decimals: they look the same.
    CHECK(report({2.0, 2.001}) == "mean 2.00, all equal");
    // But it compares numbers, not text: "10.00" sorts before "2.00".
    CHECK(report({2, 10}) == "mean 6.00, range 2.00 to 10.00");
}

TEST_CASE("report of nothing, and of one value") {
    verbosity = 1;
    CHECK(report({}).empty());
    CHECK(report({7}) == "mean 7.00, all equal");
}

TEST_CASE("verbosity is one variable everywhere") {
    verbosity = 0;
    CHECK(report({-1, 1}) == "mean 0.00");
    verbosity = 1;
    CHECK(report({-1, 1}) == "mean 0.00, range -1.00 to 1.00");
}

TEST_CASE("mean of nothing") {
    CHECK(mean({}) == 0);
}
