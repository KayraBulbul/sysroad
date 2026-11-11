#include "doctest.h"

#include <algorithm>
#include <vector>

#include "branches.hpp"

TEST_CASE("filter_above keeps the order, and handles nothing to keep") {
    std::vector<int> in(1000);
    for (std::size_t i = 0; i < in.size(); ++i) {
        in[i] = static_cast<int>(i * 7919 % 1000) - 500;
    }
    for (int limit : {-1000, -1, 0, 250, 499, 1000}) {
        std::vector<int> want;
        std::copy_if(in.begin(), in.end(), std::back_inserter(want),
                     [&](int x) { return x > limit; });
        std::vector<int> out(in.size());
        std::size_t k = filter_above(in.data(), in.size(), limit, out.data());
        out.resize(k);
        CAPTURE(limit);
        CHECK(out == want);
    }
    std::vector<int> none;
    CHECK(filter_above(none.data(), 0, 0, none.data()) == 0);
}

TEST_CASE("lower_bound_index matches std::lower_bound for every size") {
    for (std::size_t n = 0; n <= 40; ++n) {
        std::vector<int> a(n);
        for (std::size_t i = 0; i < n; ++i) {
            a[i] = static_cast<int>(3 * i);
        }
        for (int x = -2; x <= static_cast<int>(3 * n + 2); ++x) {
            std::size_t want =
                static_cast<std::size_t>(std::lower_bound(a.begin(), a.end(), x) - a.begin());
            CAPTURE(n);
            CAPTURE(x);
            CHECK(lower_bound_index(a.data(), n, x) == want);
        }
    }
}

TEST_CASE("lower_bound_index finds the first of equal elements") {
    std::vector<int> a{1, 2, 2, 2, 2, 3, 3, 9};
    CHECK(lower_bound_index(a.data(), a.size(), 2) == 1);
    CHECK(lower_bound_index(a.data(), a.size(), 3) == 5);
    CHECK(lower_bound_index(a.data(), a.size(), 4) == 7);
    std::vector<int> same(100, 7);
    CHECK(lower_bound_index(same.data(), same.size(), 7) == 0);
    CHECK(lower_bound_index(same.data(), same.size(), 8) == 100);
}
