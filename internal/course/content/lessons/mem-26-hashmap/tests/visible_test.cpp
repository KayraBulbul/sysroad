#include "doctest.h"

#include <string>

#include "hashes.hpp"
#include "hashmap.hpp"

TEST_CASE("insert and find") {
    HashMap<std::string, int> m;
    CHECK(m.insert("a", 1));
    CHECK(m.insert("b", 2));
    REQUIRE(m.find("a") != nullptr);
    CHECK(*m.find("a") == 1);
    CHECK(*m.find("b") == 2);
    CHECK(m.find("c") == nullptr);
    CHECK(m.size() == 2);
}

TEST_CASE("a key is only inserted once") {
    HashMap<std::string, int> m;
    CHECK(m.insert("k", 1));
    CHECK_FALSE(m.insert("k", 2));
    CHECK(*m.find("k") == 1);
    CHECK(m.size() == 1);
}

TEST_CASE("collisions probe onward") {
    HashMap<int, int, ZeroHash> m;
    for (int i = 0; i < 5; ++i) {
        m.insert(i, i * 10);
    }
    for (int i = 0; i < 5; ++i) {
        REQUIRE(m.find(i) != nullptr);
        CHECK(*m.find(i) == i * 10);
    }
    CHECK(m.find(5) == nullptr);
}

TEST_CASE("operator[] counts") {
    HashMap<std::string, int> m;
    for (const char* w : {"x", "y", "x", "x"}) {
        ++m[w];
    }
    CHECK(m["x"] == 3);
    CHECK(m["y"] == 1);
    CHECK(m["z"] == 0);
    CHECK(m.size() == 3);
}
