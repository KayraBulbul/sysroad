#include "doctest.h"

#include <string>

#include "hashes.hpp"
#include "hashmap.hpp"

TEST_CASE("it grows and keeps everything") {
    HashMap<int, int> m;
    for (int i = 0; i < 1000; ++i) {
        CHECK(m.insert(i * 7, i));
    }
    CHECK(m.size() == 1000);
    CHECK(m.capacity() >= 1334);  // load stays at or below 3/4
    CHECK((m.capacity() & (m.capacity() - 1)) == 0);
    for (int i = 0; i < 1000; ++i) {
        REQUIRE(m.find(i * 7) != nullptr);
        CHECK(*m.find(i * 7) == i);
    }
    CHECK(m.find(1) == nullptr);
}

TEST_CASE("the load never passes 3/4") {
    HashMap<int, int> m(4);
    for (int i = 0; i < 100; ++i) {
        m.insert(i, i);
        CHECK(m.size() * 4 <= m.capacity() * 3);
    }
}

TEST_CASE("probing wraps around the end of the table") {
    HashMap<int, int, MinusOneHash> m(8);
    // 0 hashes to the last slot (7); 8 and 16 also land on 7 and wrap to 0 and 1.
    m.insert(0, 100);
    m.insert(8, 108);
    m.insert(16, 116);
    CHECK(m.capacity() == 8);
    CHECK(*m.find(0) == 100);
    CHECK(*m.find(8) == 108);
    CHECK(*m.find(16) == 116);
    CHECK(m.find(24) == nullptr);
}

TEST_CASE("every key colliding, through several grows") {
    HashMap<int, std::string, ZeroHash> m(2);
    for (int i = 0; i < 200; ++i) {
        m.insert(i, std::to_string(i));
    }
    CHECK(m.size() == 200);
    for (int i = 0; i < 200; ++i) {
        CHECK(*m.find(i) == std::to_string(i));
    }
}

TEST_CASE("const find and contains") {
    HashMap<std::string, int> m;
    m.insert("there", 1);
    const auto& c = m;
    REQUIRE(c.find("there") != nullptr);
    CHECK(*c.find("there") == 1);
    CHECK(c.contains("there"));
    CHECK_FALSE(c.contains("not"));
}

TEST_CASE("the capacity rounds up to a power of two") {
    CHECK(HashMap<int, int>(5).capacity() == 8);
    CHECK(HashMap<int, int>(16).capacity() == 16);
    CHECK(HashMap<int, int>(1).capacity() == 1);
    HashMap<int, int> tiny(1);
    tiny.insert(1, 1);  // must grow before it's full
    tiny.insert(2, 2);
    CHECK(*tiny.find(1) == 1);
    CHECK(*tiny.find(2) == 2);
}

TEST_CASE("operator[] references stay usable until the next insert") {
    HashMap<std::string, std::string> m;
    m["greeting"] = "hello";
    m["greeting"] += ", world";
    CHECK(*m.find("greeting") == "hello, world");
}
