#include "doctest.h"

#include <string>

#include "alloc_count.hpp"
#include "bugs.hpp"

TEST_CASE("sum_numbers: edge cases, and nothing leaks") {
    AllocScope s;
    CHECK(sum_numbers("") == 0);
    CHECK(sum_numbers("  42  ") == 42);
    CHECK(sum_numbers("-5 5 10") == 10);
    CHECK(sum_numbers("3 4x") == -1);
    CHECK(sum_numbers("x") == -1);
    CHECK(s.live() == 0);
}

TEST_CASE("shout: one line, empty, long") {
    std::string long_line = std::string(100, 'q') + "\nrest";
    std::string expected(100, 'Q');
    AllocScope s;
    CHECK(shout("") == "");
    CHECK(shout("\nsecond") == "");
    CHECK(shout("mixed Case 123") == "MIXED CASE 123");
    CHECK(shout(long_line) == expected);
    CHECK(s.live() == 0);
}

TEST_CASE("doubled_copy: negative numbers, and nothing leaks") {
    AllocScope s;
    int src[] = {-4, 0, 7, 100};
    int* d = doubled_copy(src, 4);
    CHECK(d[0] == -8);
    CHECK(d[3] == 200);
    delete[] d;
    CHECK(doubled_copy(src, 0) == nullptr);
    CHECK(s.live() == 0);
}

TEST_CASE("count_unique: nothing leaks") {
    AllocScope s;
    CHECK(count_unique("") == 0);
    CHECK(count_unique("same same same") == 1);
    CHECK(count_unique("the quick brown fox jumps over the lazy dog") == 8);
    CHECK(s.live() == 0);
}
