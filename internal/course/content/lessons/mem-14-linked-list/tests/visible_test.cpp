#include "doctest.h"

#include <vector>

#include "list.hpp"

TEST_CASE("push and pop") {
    IntList l;
    l.push_back(2);
    l.push_back(3);
    l.push_front(1);
    CHECK(l.to_vector() == std::vector<int>{1, 2, 3});
    CHECK(l.size() == 3);
    CHECK(l.pop_front() == 1);
    CHECK(l.to_vector() == std::vector<int>{2, 3});
    CHECK(l.contains(3));
    CHECK_FALSE(l.contains(1));
}

TEST_CASE("reverse") {
    IntList l;
    for (int i = 1; i <= 4; ++i) {
        l.push_back(i);
    }
    l.reverse();
    CHECK(l.to_vector() == std::vector<int>{4, 3, 2, 1});
}

TEST_CASE("a million nodes") {
    IntList l;
    for (int i = 0; i < 1'000'000; ++i) {
        l.push_back(i);
    }
    CHECK(l.size() == 1'000'000);
    // ...and the destructor runs here.
}
