#pragma once
// These are templates, so the tests can pass a matrix that counts cache
// misses as well as a plain Matrix. M has rows(), cols() and (i, j).

#include <algorithm>
#include <cstddef>

// out = the transpose of a. out is a.cols() x a.rows().
template <typename M> void transpose(const M& a, M& out) {
    for (std::size_t i = 0; i < a.rows(); ++i) {
        for (std::size_t j = 0; j < a.cols(); ++j) {
            out(j, i) = a(i, j);
        }
    }
}

// The same, one block x block tile at a time.
template <typename M> void transpose_blocked(const M& a, M& out, std::size_t block) {
    // TODO
    (void)a;
    (void)out;
    (void)block;
}

// c = a × b, for n x n matrices, as the textbook writes it.
template <typename M> void multiply_ijk(const M& a, const M& b, M& c) {
    std::size_t n = a.rows();
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            double sum = 0;
            for (std::size_t k = 0; k < n; ++k) {
                sum += a(i, k) * b(k, j);
            }
            c(i, j) = sum;
        }
    }
}

// c = a × b again, with the loops in i, k, j order.
template <typename M> void multiply_ikj(const M& a, const M& b, M& c) {
    // TODO
    (void)a;
    (void)b;
    (void)c;
}
