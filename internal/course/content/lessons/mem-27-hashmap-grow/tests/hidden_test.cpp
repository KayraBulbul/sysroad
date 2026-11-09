#include "doctest.h"

#include <memory>
#include <string>

#include "hashes.hpp"
#include "hashmap.hpp"

TEST_CASE("a key further along the run isn't inserted twice") {
    HashMap<int, int, ZeroHash> m(16);
    m.insert(0, 0);
    m.insert(1, 1);
    m.insert(2, 2);
    m.erase(0);  // a tombstone at the start of the run
    CHECK_FALSE(m.insert(2, 99));
    CHECK(*m.find(2) == 2);
    CHECK(m.size() == 2);
    CHECK(m.insert(0, 100));  // reuses the tombstone
    CHECK(*m.find(0) == 100);
    CHECK(m.size() == 3);
}

TEST_CASE("erase then reinsert the same key") {
    HashMap<std::string, int> m;
    for (int round = 0; round < 5; ++round) {
        CHECK(m.insert("k", round));
        CHECK(*m.find("k") == round);
        CHECK(m.erase("k"));
    }
    CHECK(m.size() == 0);
}

TEST_CASE("churn doesn't grow the table forever") {
    HashMap<int, int> m(8);
    for (int i = 0; i < 100000; ++i) {
        m.insert(i, i);
        if (i >= 3) {
            m.erase(i - 3);
        }
    }
    CHECK(m.size() == 3);
    CHECK(m.capacity() <= 32);
    CHECK(*m.find(99999) == 99999);
    CHECK(m.find(99996) == nullptr);
}

TEST_CASE("churn with every key colliding") {
    HashMap<int, int, ZeroHash> m(8);
    for (int i = 0; i < 2000; ++i) {
        m.insert(i, i);
        m.erase(i);
    }
    CHECK(m.size() == 0);
    m.insert(5, 5);
    CHECK(*m.find(5) == 5);
}

TEST_CASE("erase frees the value") {
    HashMap<int, std::shared_ptr<int>> m;
    auto p = std::make_shared<int>(42);
    m.insert(1, p);
    CHECK(p.use_count() == 2);
    m.erase(1);
    CHECK(p.use_count() == 1);
}

TEST_CASE("rehash keeps everything and drops tombstones") {
    HashMap<int, std::string> m;
    for (int i = 0; i < 50; ++i) {
        m.insert(i, std::to_string(i));
    }
    for (int i = 0; i < 50; i += 2) {
        m.erase(i);
    }
    m.rehash(256);
    CHECK(m.capacity() == 256);
    CHECK(m.size() == 25);
    for (int i = 1; i < 50; i += 2) {
        CHECK(*m.find(i) == std::to_string(i));
    }
    m.rehash(1);  // too small for 25 entries: it picks a size that fits
    CHECK(m.size() * 4 <= m.capacity() * 3);
    CHECK(*m.find(49) == "49");
}

TEST_CASE("operator[] after erase") {
    HashMap<std::string, int> m;
    m["x"] = 5;
    m.erase("x");
    CHECK(m["x"] == 0);
    CHECK(m.size() == 1);
}
