#pragma once
// A rows × cols grid of doubles. Fix it so copies and moves are safe.

#include <cstddef>

class Matrix {
  public:
    Matrix(std::size_t rows, std::size_t cols);  // all zeros
    ~Matrix();

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
    double* data_;
};

Matrix identity(std::size_t n);
Matrix transpose(const Matrix& m);
Matrix multiply(const Matrix& a, const Matrix& b);  // a.cols() == b.rows()
