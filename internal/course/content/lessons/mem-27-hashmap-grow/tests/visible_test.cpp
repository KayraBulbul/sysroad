#include "doctest.h"

#include <set>
#include <string>

#include "hashes.hpp"
#include "hashmap.hpp"

TEST_CASE("erase") {
    HashMap<std::string, int> m;
    m.insert("a", 1);
    m.insert("b", 2);
    CHECK(m.erase("a"));
    CHECK_FALSE(m.erase("a"));
    CHECK(m.find("a") == nullptr);
    CHECK(*m.find("b") == 2);
    CHECK(m.size() == 1);
}

TEST_CASE("erasing in the middle of a run keeps the rest findable") {
    HashMap<int, int, ZeroHash> m(16);
    for (int i = 0; i < 6; ++i) {
        m.insert(i, i);
    }
    m.erase(2);
    for (int i : {0, 1, 3, 4, 5}) {
        REQUIRE(m.find(i) != nullptr);
        CHECK(*m.find(i) == i);
    }
}

TEST_CASE("for_each visits every entry") {
    HashMap<int, int> m;
    for (int i = 0; i < 20; ++i) {
        m.insert(i, i * i);
    }
    m.erase(7);
    std::set<int> seen;
    int sum = 0;
    m.for_each([&](int k, int v) {
        seen.insert(k);
        sum += v - k * k;
    });
    CHECK(seen.size() == 19);
    CHECK(seen.count(7) == 0);
    CHECK(sum == 0);
}
