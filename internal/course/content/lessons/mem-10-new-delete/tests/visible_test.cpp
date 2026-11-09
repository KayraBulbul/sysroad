#include "doctest.h"

#include "alloc_count.hpp"
#include "heap.hpp"

TEST_CASE("make_filled") {
    AllocScope s;
    int* a = make_filled(4, 9);
    long news = s.news();
    std::size_t bytes = s.bytes();
    REQUIRE(a != nullptr);
    CHECK(a[0] == 9);
    CHECK(a[3] == 9);
    delete[] a;
    CHECK(news == 1);
    CHECK(bytes == 4 * sizeof(int));
}

TEST_CASE("grow keeps the values and frees the old array") {
    int* a = make_filled(3, 5);
    AllocScope s;
    int* b = grow(a, 3, 6);
    long news = s.news(), deletes = s.deletes();
    CHECK(b[0] == 5);
    CHECK(b[2] == 5);
    CHECK(b[3] == 0);
    CHECK(b[5] == 0);
    delete[] b;
    CHECK(news == 1);
    CHECK(deletes == 1);
}

TEST_CASE("reserve means one allocation") {
    CHECK(vector_allocations(1000, true) == 1);
    CHECK(vector_allocations(1000, false) > 5);
}
