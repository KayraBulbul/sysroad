#include "doctest.h"

#include <utility>
#include <vector>

#include "alloc_count.hpp"
#include "matrix.hpp"

TEST_CASE("self-assignment") {
    Matrix a = identity(2);
    Matrix& same = a;
    a = same;
    CHECK(a.at(1, 1) == 1);
    CHECK(a.rows() == 2);
}

TEST_CASE("move assignment doesn't allocate, and the source can be reused") {
    Matrix a = identity(3);
    Matrix b(2, 2);
    AllocScope s;
    b = std::move(a);
    CHECK(s.news() == 0);
    CHECK(b.rows() == 3);
    CHECK(b.at(2, 2) == 1);
    a = identity(1);  // a moved-from object can be assigned to
    CHECK(a.at(0, 0) == 1);
}

TEST_CASE("nothing leaks through copies, moves and a growing vector") {
    AllocScope s;
    {
        std::vector<Matrix> v;
        Matrix m = identity(3);
        for (int i = 0; i < 20; ++i) {
            v.push_back(m);
            v.push_back(identity(2));
        }
        Matrix x = v[5];
        x = v[6];
        std::swap(v[0], v[1]);
        CHECK(v[0].rows() == 2);
        CHECK(v[1].rows() == 3);
    }
    CHECK(s.live() == 0);
}

TEST_CASE("the maths still works") {
    Matrix a(2, 3);
    int n = 1;
    for (std::size_t r = 0; r < 2; ++r) {
        for (std::size_t c = 0; c < 3; ++c) {
            a.at(r, c) = n++;
        }
    }
    Matrix p = multiply(a, transpose(a));
    CHECK(p.rows() == 2);
    CHECK(p.cols() == 2);
    CHECK(p.at(0, 0) == 14);
    CHECK(p.at(0, 1) == 32);
    CHECK(p.at(1, 1) == 77);
    Matrix i = multiply(identity(2), p);
    CHECK(i.at(1, 0) == 32);
}

TEST_CASE("a copy allocates once") {
    Matrix a = identity(5);
    AllocScope s;
    Matrix b = a;
    CHECK(s.news() == 1);
}
