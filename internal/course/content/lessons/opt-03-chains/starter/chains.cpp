#include "chains.hpp"

double sum1(const double* a, std::size_t n) {
    double total = 0;
    for (std::size_t i = 0; i < n; ++i) {
        total += a[i];
    }
    return total;
}

double sum2(const double* a, std::size_t n) {
    // TODO
    (void)a;
    (void)n;
    return 0;
}

double sum4(const double* a, std::size_t n) {
    // TODO
    (void)a;
    (void)n;
    return 0;
}

double poly_horner(const double* a, std::size_t n, double x) {
    // ((a[n-1]·x + a[n-2])·x + a[n-3])·x + … + a[0]
    double result = 0;
    for (std::size_t i = n; i-- > 0;) {
        result = a[i] + x * result;
    }
    return result;
}

double poly_split(const double* a, std::size_t n, double x) {
    // TODO
    (void)a;
    (void)n;
    (void)x;
    return 0;
}
