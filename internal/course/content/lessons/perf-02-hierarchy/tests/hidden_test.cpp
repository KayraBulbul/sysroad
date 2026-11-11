#include "doctest.h"

#include "cache.hpp"

TEST_CASE("set and tag for a typical L1") {
    Cache c({32 * 1024, 64, 8});
    CHECK(c.set_of(0) == 0);
    CHECK(c.set_of(63) == 0);
    CHECK(c.set_of(64) == 1);
    CHECK(c.set_of(64 * 64) == 0);
    CHECK(c.tag_of(64 * 64) == 1);
    CHECK(c.set_of(0x7ffe1234) == 0x8);
    CHECK(c.tag_of(0x7ffe1234) == 0x7ffe1234 / 4096);
}

TEST_CASE("a hit makes a line recently used") {
    Cache c({1024, 16, 2});
    c.access(0);
    c.access(512);
    c.access(0);     // now 512 is the least recently used
    c.access(1024);  // evicts 512
    CHECK(c.access(0));
    CHECK_FALSE(c.access(512));
}

TEST_CASE("direct-mapped: two lines in the same set keep evicting each other") {
    Cache c({1024, 16, 1});
    for (int i = 0; i < 10; ++i) {
        CHECK_FALSE(c.access(0));
        CHECK_FALSE(c.access(1024));
    }
    CHECK(c.hits() == 0);
    CHECK(c.misses() == 20);
}

TEST_CASE("fully associative: one set, any line anywhere") {
    Cache c({1024, 16, 64});
    CHECK(c.sets() == 1);
    for (std::uint64_t a = 0; a < 1024; a += 16) {
        c.access(a * 1000);
    }
    for (std::uint64_t a = 0; a < 1024; a += 16) {
        CHECK(c.access(a * 1000));
    }
    CHECK_FALSE(c.access(1 << 30));  // a 65th line evicts the oldest, address 0
    CHECK_FALSE(c.access(0));
}

TEST_CASE("sets don't share lines") {
    Cache c({1024, 16, 2});
    // Fill set 0, then use set 1. Set 0 keeps both its lines.
    c.access(0);
    c.access(512);
    for (std::uint64_t a = 16; a < 16 * 1000; a += 512) {
        c.access(a);
    }
    CHECK(c.access(0));
    CHECK(c.access(512));
}

TEST_CASE("a cache that fits the data misses once per line") {
    Cache c({4096, 64, 4});
    for (int pass = 0; pass < 5; ++pass) {
        for (std::uint64_t a = 0; a < 4096; a += 8) {
            c.access(a);
        }
    }
    CHECK(c.misses() == 64);
    CHECK(c.hits() == 5 * 512 - 64);
}

TEST_CASE("LRU with a loop one line too big misses every line") {
    Cache c({4096, 64, 64});  // fully associative, 64 lines
    for (int pass = 0; pass < 3; ++pass) {
        for (std::uint64_t a = 0; a < 65 * 64; a += 64) {
            c.access(a);
        }
    }
    CHECK(c.hits() == 0);
}
