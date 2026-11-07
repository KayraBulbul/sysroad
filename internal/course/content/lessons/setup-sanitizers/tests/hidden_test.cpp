#include "doctest.h"

#include <climits>
#include <vector>

#include "bugs.hpp"

TEST_CASE("sum_first: n = 0") {
    std::vector<int> v{5};
    CHECK(sum_first(v, 0) == 0);
}

TEST_CASE("average: negatives and mixed signs") {
    CHECK(average(INT_MIN, INT_MIN) == INT_MIN);
    CHECK(average(-3, 4) == 0);
    CHECK(average(INT_MIN, INT_MAX) == 0);
}

TEST_CASE("count_digit: letters and punctuation are ignored") {
    CHECK(count_digit("a1b1:1/9", 1) == 3);
    CHECK(count_digit("", 5) == 0);
    CHECK(count_digit("x\xff\x80 9", 9) == 1);
}

TEST_CASE("first_then_grow: many times") {
    std::vector<int> v{1};
    for (int i = 0; i < 100; ++i) {
        CHECK(first_then_grow(v) == 1);
    }
}
