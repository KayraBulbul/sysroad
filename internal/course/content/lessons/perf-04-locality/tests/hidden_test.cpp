#include "doctest.h"

#include "locality.hpp"
#include "matrix.hpp"
#include "traced.hpp"

static Matrix numbered(std::size_t rows, std::size_t cols) {
    Matrix m(rows, cols);
    for (std::size_t i = 0; i < rows; ++i) {
        for (std::size_t j = 0; j < cols; ++j) {
            m(i, j) = static_cast<double>(i * 100 + j) / 7;
        }
    }
    return m;
}

static const CacheConfig tiny{4096, 64, 4};

TEST_CASE("transpose_blocked when the block doesn't divide the size") {
    for (std::size_t block : {1, 3, 4, 5, 8, 64}) {
        Matrix a = numbered(10, 7);
        Matrix want(7, 10), got(7, 10);
        transpose(a, want);
        transpose_blocked(a, got, block);
        CAPTURE(block);
        CHECK(got == want);
    }
}

TEST_CASE("transpose_blocked of a single row and a single column") {
    Matrix row = numbered(1, 9), col = numbered(9, 1);
    Matrix row_want(9, 1), col_want(1, 9), row_got(9, 1), col_got(1, 9);
    transpose(row, row_want);
    transpose(col, col_want);
    transpose_blocked(row, row_got, 4);
    transpose_blocked(col, col_got, 4);
    CHECK(row_got == row_want);
    CHECK(col_got == col_want);
}

TEST_CASE("multiply_ikj overwrites what was in c") {
    Matrix a = numbered(5, 5), b = numbered(5, 5);
    Matrix want(5, 5), got(5, 5);
    for (std::size_t i = 0; i < 5; ++i) {
        for (std::size_t j = 0; j < 5; ++j) {
            got(i, j) = 1e9;
        }
    }
    multiply_ijk(a, b, want);
    multiply_ikj(a, b, got);
    for (std::size_t i = 0; i < 5; ++i) {
        for (std::size_t j = 0; j < 5; ++j) {
            CHECK(got(i, j) == doctest::Approx(want(i, j)));
        }
    }
}

TEST_CASE("multiply_ikj of 1x1 and 17x17") {
    for (std::size_t n : {1, 17}) {
        Matrix a = numbered(n, n), b = numbered(n, n);
        Matrix want(n, n), got(n, n);
        multiply_ijk(a, b, want);
        multiply_ikj(a, b, got);
        for (std::size_t i = 0; i < n; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                CHECK(got(i, j) == doctest::Approx(want(i, j)));
            }
        }
    }
}

TEST_CASE("blocking beats power-of-two conflicts") {
    // 64 doubles is 512 bytes, so every row of a column lands in the same
    // few sets.
    Cache naive(tiny), blocked(tiny);
    TracedMatrix a1(64, 64, naive, 1ull << 32), out1(64, 64, naive, 2ull << 32);
    TracedMatrix a2(64, 64, blocked, 1ull << 32), out2(64, 64, blocked, 2ull << 32);
    transpose(a1, out1);
    transpose_blocked(a2, out2, 8);
    CAPTURE(naive.misses());
    CHECK(blocked.misses() * 2 < naive.misses());
}

TEST_CASE("multiply_ikj keeps its advantage on a bigger matrix") {
    Cache ijk(tiny), ikj(tiny);
    TracedMatrix a1(72, 72, ijk, 1ull << 32), b1(72, 72, ijk, 2ull << 32),
        c1(72, 72, ijk, 3ull << 32);
    TracedMatrix a2(72, 72, ikj, 1ull << 32), b2(72, 72, ikj, 2ull << 32),
        c2(72, 72, ikj, 3ull << 32);
    multiply_ijk(a1, b1, c1);
    multiply_ikj(a2, b2, c2);
    CAPTURE(ijk.misses());
    CHECK(ikj.misses() * 4 <= ijk.misses());
}
