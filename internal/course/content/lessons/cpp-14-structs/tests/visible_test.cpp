#include "doctest.h"

#include "histogram.hpp"

TEST_CASE("add and count") {
    Histogram h;
    h.add("hello");
    CHECK(h.count('l') == 2);
    CHECK(h.count('h') == 1);
    CHECK(h.count('z') == 0);
    CHECK(h.total() == 5);
}

TEST_CASE("most_common") {
    Histogram h;
    h.add("hello");
    CHECK(h.most_common() == 'l');
}
