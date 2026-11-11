#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstddef>
#include <vector>

// A rows x cols matrix of doubles, all zero to start, stored row after
// row: element (i, j) is at index i * cols + j.
class Matrix {
  public:
    Matrix(std::size_t rows, std::size_t cols) : rows_(rows), cols_(cols), v_(rows * cols) {}

    std::size_t rows() const {
        return rows_;
    }
    std::size_t cols() const {
        return cols_;
    }
    double& operator()(std::size_t i, std::size_t j) {
        return v_[i * cols_ + j];
    }
    const double& operator()(std::size_t i, std::size_t j) const {
        return v_[i * cols_ + j];
    }
    bool operator==(const Matrix&) const = default;

  private:
    std::size_t rows_;
    std::size_t cols_;
    std::vector<double> v_;
};
