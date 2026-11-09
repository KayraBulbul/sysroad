#pragma once
// A rows × cols grid of doubles. Fix it so copies and moves are safe.

#include <cstddef>
#include <vector>

// The rule of zero: the vector owns the numbers, so the compiler's copies
// and moves are right, and there's no destructor to write.
class Matrix {
  public:
    Matrix(std::size_t rows, std::size_t cols);  // all zeros

    std::size_t rows() const {
        return rows_;
    }
    std::size_t cols() const {
        return cols_;
    }
    double& at(std::size_t r, std::size_t c) {
        return data_[r * cols_ + c];
    }
    double at(std::size_t r, std::size_t c) const {
        return data_[r * cols_ + c];
    }

  private:
    std::size_t rows_;
    std::size_t cols_;
    std::vector<double> data_;
};

Matrix identity(std::size_t n);
Matrix transpose(const Matrix& m);
Matrix multiply(const Matrix& a, const Matrix& b);  // a.cols() == b.rows()
