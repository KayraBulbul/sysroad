#include "doctest.h"

#include <climits>
#include <vector>

#include "bugs.hpp"

// One test per bug, in order. Under AddressSanitizer a bad memory access
// stops the whole test run with a report, so fix them one at a time.

TEST_CASE("1. sum_first") {
    std::vector<int> v{1, 2, 3, 4};
    CHECK(sum_first(v, 4) == 10);
    CHECK(sum_first(v, 2) == 3);
}

TEST_CASE("2. first_then_grow") {
    std::vector<int> v{7, 8, 9};
    v.shrink_to_fit();  // full, so the push_back has to move the elements
    CHECK(first_then_grow(v) == 7);
    CHECK(v.size() == 4);
}

TEST_CASE("3. average") {
    CHECK(average(4, 8) == 6);
    CHECK(average(INT_MAX, INT_MAX - 2) == INT_MAX - 1);
}

TEST_CASE("4. welcome") {
    CHECK(welcome("Kayra") == "Hello, Kayra! Welcome to the systems course.");
}

TEST_CASE("5. count_digit") {
    CHECK(count_digit("2026-11-07", 0) == 2);
}
