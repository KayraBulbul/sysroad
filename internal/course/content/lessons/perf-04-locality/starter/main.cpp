// Times the textbook loops against yours on real matrices. You won't need
// to change this file.
#include <cstdio>
#include <functional>

#include "bench.hpp"
#include "locality.hpp"
#include "matrix.hpp"

static Matrix filled(std::size_t n) {
    Matrix m(n, n);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            m(i, j) = static_cast<double>((i * 7 + j * 3) % 10);
        }
    }
    return m;
}

// The fastest of 3 runs, in milliseconds.
static double best_ms(const std::function<void()>& f) {
    return measure(f, 3, 0).min / 1e6;
}

static void race(const char* what, double textbook, double yours, bool same) {
    if (!same) {
        std::printf("%-30s %9.1f ms   (yours gives a different answer)\n", what, textbook);
        return;
    }
    std::printf("%-30s %9.1f ms %9.1f ms   %.1fx\n", what, textbook, yours, textbook / yours);
}

int main() {
    std::printf("%-30s %12s %12s\n", "", "textbook", "yours");

    for (std::size_t n : {1000, 1024, 4096}) {
        Matrix a = filled(n);
        Matrix want(n, n), out(n, n);
        double naive = best_ms([&] {
            transpose(a, want);
            do_not_optimize(want(1, 2));
        });
        double blocked = best_ms([&] {
            transpose_blocked(a, out, 8);
            do_not_optimize(out(1, 2));
        });
        char what[64];
        std::snprintf(what, sizeof what, "transpose %zux%zu", n, n);
        race(what, naive, blocked, out == want);
    }

    for (std::size_t n : {500, 512}) {
        Matrix a = filled(n);
        Matrix b = filled(n);
        Matrix want(n, n), c(n, n);
        double ijk = best_ms([&] {
            multiply_ijk(a, b, want);
            do_not_optimize(want(1, 2));
        });
        double ikj = best_ms([&] {
            multiply_ikj(a, b, c);
            do_not_optimize(c(1, 2));
        });
        char what[64];
        std::snprintf(what, sizeof what, "multiply %zux%zu", n, n);
        race(what, ijk, ikj, c == want);
    }
}
