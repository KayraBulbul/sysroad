#include "doctest.h"

#include <algorithm>
#include <string>
#include <vector>

#include "vector.hpp"

static std::vector<int> as_std(const Vector<int>& v) {
    return std::vector<int>(v.begin(), v.end());
}

TEST_CASE("begin, end, range-for and std::sort") {
    Vector<int> v = {3, 1, 2};
    std::sort(v.begin(), v.end());
    CHECK(as_std(v) == std::vector<int>{1, 2, 3});
    int sum = 0;
    for (int x : v) {
        sum += x;
    }
    CHECK(sum == 6);
    CHECK(std::find(v.begin(), v.end(), 9) == v.end());
}

TEST_CASE("insert") {
    Vector<int> v = {1, 3};
    auto it = v.insert(v.begin() + 1, 2);
    CHECK(*it == 2);
    v.insert(v.begin(), 0);
    v.insert(v.end(), 4);
    CHECK(as_std(v) == std::vector<int>{0, 1, 2, 3, 4});
}

TEST_CASE("erase") {
    Vector<int> v = {0, 1, 2, 3, 4};
    auto it = v.erase(v.begin() + 1);
    CHECK(*it == 2);
    CHECK(as_std(v) == std::vector<int>{0, 2, 3, 4});
    it = v.erase(v.begin() + 1, v.begin() + 3);
    CHECK(*it == 4);
    CHECK(as_std(v) == std::vector<int>{0, 4});
}

TEST_CASE("erase_if") {
    Vector<int> v = {1, 2, 3, 4, 5, 6};
    CHECK(erase_if(v, [](int x) { return x % 2 == 0; }) == 3);
    CHECK(as_std(v) == std::vector<int>{1, 3, 5});
}
