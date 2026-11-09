#include "doctest.h"

#include <vector>

#include "list.hpp"

TEST_CASE("tail stays right through pops") {
    IntList l;
    l.push_back(1);
    CHECK(l.pop_front() == 1);
    CHECK(l.empty());
    l.push_back(2);  // tail_ must have been reset, or this writes through a dangling pointer
    l.push_back(3);
    CHECK(l.to_vector() == std::vector<int>{2, 3});
}

TEST_CASE("push_front on an empty list sets the tail") {
    IntList l;
    l.push_front(5);
    l.push_back(6);
    CHECK(l.to_vector() == std::vector<int>{5, 6});
}

TEST_CASE("reverse fixes the tail") {
    IntList l;
    l.push_back(1);
    l.push_back(2);
    l.push_back(3);
    l.reverse();
    l.push_back(0);
    CHECK(l.to_vector() == std::vector<int>{3, 2, 1, 0});
    CHECK(l.size() == 4);
}

TEST_CASE("reverse of empty and one") {
    IntList e;
    e.reverse();
    CHECK(e.empty());
    e.push_back(1);  // still usable
    CHECK(e.to_vector() == std::vector<int>{1});
    IntList one;
    one.push_back(7);
    one.reverse();
    one.push_back(8);
    CHECK(one.to_vector() == std::vector<int>{7, 8});
}

TEST_CASE("a million push_fronts, reversed, then emptied") {
    IntList l;
    for (int i = 0; i < 1'000'000; ++i) {
        l.push_front(i);
    }
    l.reverse();
    CHECK(l.pop_front() == 0);
    CHECK(l.pop_front() == 1);
    CHECK(l.contains(999'999));
    CHECK_FALSE(l.contains(-1));
    CHECK(l.size() == 999'998);
}

TEST_CASE("mixed operations") {
    IntList l;
    std::vector<int> model;
    for (int i = 0; i < 100; ++i) {
        if (i % 3 == 0) {
            l.push_front(i);
            model.insert(model.begin(), i);
        } else if (i % 7 == 0 && !model.empty()) {
            CHECK(l.pop_front() == model.front());
            model.erase(model.begin());
        } else {
            l.push_back(i);
            model.push_back(i);
        }
    }
    CHECK(l.to_vector() == model);
    CHECK(l.size() == model.size());
}
