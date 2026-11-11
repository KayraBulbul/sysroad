#include "doctest.h"

#include <vector>

#include "branches.hpp"

TEST_CASE("filter_above") {
    std::vector<int> in{5, 200, 17, 130, 128, 127};
    std::vector<int> out(in.size());
    std::size_t k = filter_above(in.data(), in.size(), 127, out.data());
    REQUIRE(k == 3);
    CHECK(out[0] == 200);
    CHECK(out[1] == 130);
    CHECK(out[2] == 128);
}

TEST_CASE("lower_bound_index") {
    std::vector<int> a{10, 20, 30, 40, 50};
    CHECK(lower_bound_index(a.data(), a.size(), 30) == 2);
    CHECK(lower_bound_index(a.data(), a.size(), 31) == 3);
    CHECK(lower_bound_index(a.data(), a.size(), 5) == 0);
    CHECK(lower_bound_index(a.data(), a.size(), 99) == 5);
}
