#include "doctest.h"

#include "report.hpp"
#include "stats.hpp"

TEST_CASE("mean") {
    CHECK(mean({1, 2, 3, 4}) == 2.5);
}

TEST_CASE("report") {
    verbosity = 1;
    CHECK(report({1, 2, 3, 4}) == "mean 2.50, range 1.00 to 4.00");
    verbosity = 0;
    CHECK(report({1, 2, 3, 4}) == "mean 2.50");
    verbosity = 1;
}
