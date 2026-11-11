#include "doctest.h"

#include "cache.hpp"

// 1 KB, 16-byte lines, 2 ways: 32 sets.
static const CacheConfig small{1024, 16, 2};

TEST_CASE("sets") {
    CHECK(Cache(small).sets() == 32);
    CHECK(Cache({32 * 1024, 64, 8}).sets() == 64);
}

TEST_CASE("an address splits into tag, set and offset") {
    Cache c(small);
    // 0x1234 is line 0x123 (offset 4). 0x123 % 32 is set 3, 0x123 / 32 is tag 9.
    CHECK(c.set_of(0x1234) == 3);
    CHECK(c.tag_of(0x1234) == 9);
    // Bytes in the same line share both.
    CHECK(c.set_of(0x1230) == 3);
    CHECK(c.tag_of(0x123f) == 9);
}

TEST_CASE("the second read of a line hits") {
    Cache c(small);
    CHECK_FALSE(c.access(100));
    CHECK(c.access(100));
    CHECK(c.access(96));  // the same 16-byte line: 96 to 111
    CHECK_FALSE(c.access(112));
    CHECK(c.hits() == 2);
    CHECK(c.misses() == 2);
}

TEST_CASE("a full set evicts its least recently used line") {
    Cache c(small);
    // 0, 512 and 1024 all go in set 0, which has 2 ways.
    c.access(0);
    c.access(512);
    c.access(1024);  // evicts 0
    CHECK_FALSE(c.access(0));
}
