#include "doctest.h"

#include <string>

#include "alloc_count.hpp"
#include "counted.hpp"
#include "vector.hpp"

TEST_CASE("push_back and grow") {
    AllocScope s;
    {
        Vector<int> v;
        for (int i = 0; i < 100; ++i) {
            v.push_back(i);
        }
        CHECK(v.size() == 100);
        CHECK(v.capacity() >= 100);
        CHECK(v[0] == 0);
        CHECK(v[99] == 99);
        CHECK(s.news() < 10);
    }
    CHECK(s.live() == 0);
}

TEST_CASE("every element is destroyed exactly once") {
    Counted::reset();
    {
        Vector<Counted> v;
        for (int i = 0; i < 20; ++i) {
            v.emplace_back(std::to_string(i));
        }
        CHECK(Counted::live == 20);
        CHECK(v[13].value == "13");
        v.pop_back();
        CHECK(Counted::live == 19);
    }
    CHECK(Counted::live == 0);
}

TEST_CASE("reserve doesn't construct anything") {
    Vector<NoDefault> v;
    v.reserve(10);
    CHECK(v.size() == 0);
    v.emplace_back(7);
    CHECK(v[0].n == 7);
}
