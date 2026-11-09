#include "doctest.h"

#include <array>
#include <vector>

#include "arrays.hpp"

TEST_CASE("sum of negatives and one element") {
    int v[] = {-4, 10, -6};
    CHECK(sum(v, v + 3) == 0);
    CHECK(sum(v + 1, v + 2) == 10);
}

TEST_CASE("reverse an even count, one, and none") {
    std::vector<int> v = {1, 2, 3, 4};
    reverse(v.data(), v.data() + v.size());
    CHECK(v == std::vector<int>{4, 3, 2, 1});

    int one[] = {7};
    reverse(one, one + 1);
    CHECK(one[0] == 7);

    reverse(one, one);  // empty: nothing to do, nothing to touch
    CHECK(one[0] == 7);
}

TEST_CASE("reverse only touches its range") {
    int v[] = {0, 1, 2, 3, 9};
    reverse(v + 1, v + 4);
    CHECK(v[0] == 0);
    CHECK(v[1] == 3);
    CHECK(v[3] == 1);
    CHECK(v[4] == 9);
}

TEST_CASE("doubled leaves the original alone") {
    std::array<int, 5> a = {0, -1, 7, 100, 3};
    std::array<int, 5> d = doubled(a);
    CHECK(d == std::array<int, 5>{0, -2, 14, 200, 6});
    CHECK(a == std::array<int, 5>{0, -1, 7, 100, 3});
}
