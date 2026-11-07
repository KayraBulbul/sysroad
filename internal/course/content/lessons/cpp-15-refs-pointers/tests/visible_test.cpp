#include "doctest.h"

#include <string>
#include <vector>

#include "refs.hpp"

TEST_CASE("swap_values") {
    int a = 1, b = 2;
    swap_values(a, b);
    CHECK(a == 2);
    CHECK(b == 1);
}

TEST_CASE("find_value: found, and the pointer is into the vector") {
    std::vector<int> v{4, 8, 15};
    int* p = find_value(v, 8);
    REQUIRE(p != nullptr);
    *p = 99;
    CHECK(v[1] == 99);
}

TEST_CASE("trim") {
    std::string s = "  hello  ";
    trim(s);
    CHECK(s == "hello");
}
