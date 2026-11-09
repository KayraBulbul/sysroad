#include "doctest.h"

#include <utility>

#include "alloc_count.hpp"
#include "matrix.hpp"

TEST_CASE("a copy is independent") {
    Matrix a = identity(2);
    Matrix b = a;
    b.at(0, 0) = 5;
    CHECK(a.at(0, 0) == 1);
    CHECK(b.at(0, 0) == 5);
}

TEST_CASE("assignment copies") {
    Matrix a = identity(3);
    Matrix b(1, 1);
    b = a;
    CHECK(b.rows() == 3);
    b.at(2, 2) = 7;
    CHECK(a.at(2, 2) == 1);
}

TEST_CASE("moving doesn't allocate") {
    Matrix a = identity(4);
    AllocScope s;
    Matrix b = std::move(a);
    CHECK(s.news() == 0);
    CHECK(b.at(3, 3) == 1);
}
