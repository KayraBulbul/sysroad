#include "doctest.h"

#include "alloc_count.hpp"
#include "heap.hpp"

TEST_CASE("grow can shrink") {
    int* a = make_filled(5, 1);
    a[4] = 2;
    int* b = grow(a, 5, 2);
    CHECK(b[0] == 1);
    CHECK(b[1] == 1);
    delete[] b;
}

TEST_CASE("grow from nothing") {
    AllocScope s;
    int* b = grow(nullptr, 0, 3);
    CHECK(b[0] == 0);
    CHECK(b[2] == 0);
    delete[] b;
    CHECK(s.live() == 0);
}

TEST_CASE("nothing leaks") {
    AllocScope s;
    int* a = make_filled(10, 3);
    a = grow(a, 10, 20);
    a = grow(a, 20, 40);
    delete[] a;
    vector_allocations(100, false);
    vector_allocations(100, true);
    CHECK(s.live() == 0);
}

TEST_CASE("vector growth is logarithmic") {
    CHECK(vector_allocations(0, false) == 0);
    CHECK(vector_allocations(1, false) == 1);
    long thousand = vector_allocations(1000, false);
    long million = vector_allocations(1000000, false);
    CHECK(thousand >= 10);
    CHECK(thousand <= 20);
    CHECK(million - thousand >= 9);  // about log2(1000) more
    CHECK(million <= 40);
    CHECK(vector_allocations(1000000, true) == 1);
}
