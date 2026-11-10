#include "doctest.h"

#include <vector>

#include "conv.hpp"

TEST_CASE("below compares numbers, not bit patterns") {
    CHECK(below(3, 5));
    CHECK_FALSE(below(5, 5));
    CHECK(below(-1, 5));
}

TEST_CASE("last_index_of") {
    CHECK(last_index_of({4, 7, 4, 9}, 4) == 2);
    CHECK(last_index_of({4, 7, 4, 9}, 9) == 3);
    CHECK(last_index_of({4, 7, 4, 9}, 5) == -1);
}

TEST_CASE("byte_histogram counts every byte value") {
    auto counts = byte_histogram("caf\xc3\xa9");
    CHECK(counts['c'] == 1);
    CHECK(counts[0xc3] == 1);
    CHECK(counts[0xa9] == 1);
}

TEST_CASE("average") {
    CHECK(average(10, 20) == 15);
    CHECK(average(200, 100) == 150);
}

TEST_CASE("mib_to_bytes") {
    CHECK(mib_to_bytes(1) == 1048576);
    CHECK(mib_to_bytes(8192) == 8589934592);
}

TEST_CASE("bit") {
    CHECK(bit(0) == 1);
    CHECK(bit(31) == 0x80000000);
    CHECK(bit(40) == 0x10000000000);
}
