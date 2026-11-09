#include "doctest.h"

#include "alloc_count.hpp"
#include "bugs.hpp"

TEST_CASE("sum_numbers doesn't leak") {
    AllocScope s;
    CHECK(sum_numbers("1 2 3") == 6);
    CHECK(sum_numbers("1 two 3") == -1);
    CHECK(s.live() == 0);
}

TEST_CASE("shout") {
    CHECK(shout("hello\nworld") == "HELLO");
}

TEST_CASE("doubled_copy") {
    int src[] = {1, 2, 3};
    int* d = doubled_copy(src, 3);
    CHECK(d[2] == 6);
    delete[] d;
    d = doubled_copy(src, 0);
    CHECK(d == nullptr);
    delete[] d;
}

TEST_CASE("count_unique") {
    CHECK(count_unique("a b a c b") == 3);
}
