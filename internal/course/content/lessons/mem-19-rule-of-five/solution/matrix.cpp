#include "matrix.hpp"

Matrix::Matrix(std::size_t rows, std::size_t cols)
    : rows_(rows), cols_(cols), data_(rows * cols, 0.0) {}

Matrix identity(std::size_t n) {
    Matrix m(n, n);
    for (std::size_t i = 0; i < n; ++i) {
        m.at(i, i) = 1;
    }
    return m;
}

Matrix transpose(const Matrix& m) {
    Matrix t(m.cols(), m.rows());
    for (std::size_t r = 0; r < m.rows(); ++r) {
        for (std::size_t c = 0; c < m.cols(); ++c) {
            t.at(c, r) = m.at(r, c);
        }
    }
    return t;
}

Matrix multiply(const Matrix& a, const Matrix& b) {
    Matrix out(a.rows(), b.cols());
    for (std::size_t r = 0; r < a.rows(); ++r) {
        for (std::size_t c = 0; c < b.cols(); ++c) {
            double sum = 0;
            for (std::size_t k = 0; k < a.cols(); ++k) {
                sum += a.at(r, k) * b.at(k, c);
            }
            out.at(r, c) = sum;
        }
    }
    return out;
}
