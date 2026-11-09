#include "doctest.h"

#include <array>

#include "arrays.hpp"

TEST_CASE("sum") {
    int v[] = {1, 2, 3, 4, 5};
    CHECK(sum(v, v + 5) == 15);
    CHECK(sum(v + 1, v + 4) == 9);
    CHECK(sum(v, v) == 0);
}

TEST_CASE("reverse") {
    int v[] = {1, 2, 3, 4, 5};
    reverse(v, v + 5);
    CHECK(v[0] == 5);
    CHECK(v[2] == 3);
    CHECK(v[4] == 1);
}

TEST_CASE("doubled") {
    std::array<int, 5> a = {1, 2, 3, 4, 5};
    CHECK(doubled(a) == std::array<int, 5>{2, 4, 6, 8, 10});
}
