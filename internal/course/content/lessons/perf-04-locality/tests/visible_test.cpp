#include "doctest.h"

#include "locality.hpp"
#include "matrix.hpp"
#include "traced.hpp"

static Matrix numbered(std::size_t rows, std::size_t cols) {
    Matrix m(rows, cols);
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < cols; ++j) {
            m(i, j) = static_cast<double>(i * 100 + j);
        }
    }
    return m;
}

// The tests use a small cache, 4 KB with 4 ways, so small matrices
// already don't fit in it.
static const CacheConfig tiny{4096, 64, 4};

TEST_CASE("transpose_blocked gives the same answer as transpose") {
    Matrix a = numbered(6, 6);
    Matrix want(6, 6), got(6, 6);
    transpose(a, want);
    transpose_blocked(a, got, 4);
    CHECK(got == want);
}

TEST_CASE("multiply_ikj gives the same answer as multiply_ijk") {
    Matrix a = numbered(4, 4), b = numbered(4, 4);
    Matrix want(4, 4), got(4, 4);
    multiply_ijk(a, b, want);
    multiply_ikj(a, b, got);
    CHECK(got == want);
}

TEST_CASE("transpose_blocked misses less than half as often") {
    Cache naive(tiny), blocked(tiny);
    TracedMatrix a1(72, 72, naive, 1ull << 32), out1(72, 72, naive, 2ull << 32);
    TracedMatrix a2(72, 72, blocked, 1ull << 32), out2(72, 72, blocked, 2ull << 32);
    transpose(a1, out1);
    transpose_blocked(a2, out2, 8);
    CAPTURE(naive.misses());
    CHECK(blocked.misses() * 2 < naive.misses());
}

TEST_CASE("multiply_ikj misses at most a quarter as often") {
    Cache ijk(tiny), ikj(tiny);
    TracedMatrix a1(48, 48, ijk, 1ull << 32), b1(48, 48, ijk, 2ull << 32), c1(48, 48, ijk, 3ull << 32);
    TracedMatrix a2(48, 48, ikj, 1ull << 32), b2(48, 48, ikj, 2ull << 32), c2(48, 48, ikj, 3ull << 32);
    multiply_ijk(a1, b1, c1);
    multiply_ikj(a2, b2, c2);
    CAPTURE(ijk.misses());
    CHECK(ikj.misses() * 4 <= ijk.misses());
}
