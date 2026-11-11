#pragma once
// A matrix that sends every element it hands out through a simulated cache.

#include <cstdint>
#include <ostream>

#include "cache.hpp"
#include "matrix.hpp"

class TracedMatrix {
  public:
    // base is the matrix's pretend address, so results don't depend on
    // where malloc put it.
    TracedMatrix(std::size_t rows, std::size_t cols, Cache& cache, std::uint64_t base)
        : m_(rows, cols), cache_(&cache), base_(base) {}

    std::size_t rows() const {
        return m_.rows();
    }
    std::size_t cols() const {
        return m_.cols();
    }
    double& operator()(std::size_t i, std::size_t j) {
        touch(i, j);
        return m_(i, j);
    }
    const double& operator()(std::size_t i, std::size_t j) const {
        touch(i, j);
        return m_(i, j);
    }
    Matrix& plain() {
        return m_;
    }

  private:
    void touch(std::size_t i, std::size_t j) const {
        cache_->access(base_ + (i * m_.cols() + j) * sizeof(double));
    }

    Matrix m_;
    Cache* cache_;
    std::uint64_t base_;
};

// So a failing CHECK(a == b) prints the matrices.
inline std::ostream& operator<<(std::ostream& os, const Matrix& m) {
    os << "{";
    for (std::size_t i = 0; i < m.rows(); ++i) {
        os << (i ? ", {" : "{");
        for (std::size_t j = 0; j < m.cols(); ++j) {
            os << (j ? ", " : "") << m(i, j);
        }
        os << "}";
    }
    return os << "}";
}
