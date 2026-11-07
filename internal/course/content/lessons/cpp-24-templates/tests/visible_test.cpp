#include "doctest.h"

#include <string>

#include "generic.hpp"

TEST_CASE("max_of: ints and strings") {
    CHECK(max_of(std::vector<int>{3, 9, 4}) == 9);
    CHECK(max_of(std::vector<std::string>{"pear", "apple"}) == "pear");
}

TEST_CASE("Stack: last in, first out") {
    Stack<int> s;
    s.push(1);
    s.push(2);
    CHECK(s.size() == 2);
    CHECK(s.pop() == 2);
    CHECK(s.pop() == 1);
    CHECK(s.empty());
}
