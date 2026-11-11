#include "doctest.h"

#include <vector>

#include "chains.hpp"

// Whole numbers add up exactly in any order, so these can use ==.

TEST_CASE("sum2") {
    std::vector<double> a{1, 2, 3, 4, 5, 6};
    CHECK(sum2(a.data(), a.size()) == 21);
}

TEST_CASE("sum4") {
    std::vector<double> a{1, 2, 3, 4, 5, 6, 7, 8};
    CHECK(sum4(a.data(), a.size()) == 36);
}

TEST_CASE("poly_split") {
    // 1 + 2x + 3x² + 4x³ at x = 2: 1 + 4 + 12 + 32
    std::vector<double> a{1, 2, 3, 4};
    CHECK(poly_split(a.data(), a.size(), 2) == 49);
}
