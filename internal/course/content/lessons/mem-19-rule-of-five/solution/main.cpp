// Copies and moves some matrices. You won't need to change this file.
#include <cstdio>
#include <utility>
#include <vector>

#include "alloc_count.hpp"
#include "matrix.hpp"

static void print(const char* label, const Matrix& m) {
    std::printf("%s\n", label);
    for (std::size_t r = 0; r < m.rows(); ++r) {
        std::printf("  ");
        for (std::size_t c = 0; c < m.cols(); ++c) {
            std::printf("%5.1f", m.at(r, c));
        }
        std::printf("\n");
    }
}

int main() {
    Matrix a(2, 3);
    for (std::size_t c = 0; c < 3; ++c) {
        a.at(0, c) = double(c + 1);
        a.at(1, c) = double(c + 4);
    }
    print("a:", a);
    print("a times its transpose:", multiply(a, transpose(a)));

    Matrix b = a;  // a copy
    b.at(0, 0) = 99;
    std::printf("\nafter b = a; b(0,0) = 99:  a(0,0) is %.0f, b(0,0) is %.0f\n", a.at(0, 0),
                b.at(0, 0));

    AllocScope s;
    Matrix c = std::move(b);
    std::printf("moving b into c: %ld allocation%s\n", s.news(), s.news() == 1 ? "" : "s");

    std::vector<Matrix> many;
    for (int i = 0; i < 4; ++i) {
        many.push_back(identity(3));
    }
    std::printf("a vector of %zu identity matrices, trace of the last: %.0f\n", many.size(),
                many.back().at(0, 0) + many.back().at(1, 1) + many.back().at(2, 2));
}
