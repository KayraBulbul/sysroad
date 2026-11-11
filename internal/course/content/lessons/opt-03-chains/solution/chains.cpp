#include "chains.hpp"

double sum1(const double* a, std::size_t n) {
    double total = 0;
    for (std::size_t i = 0; i < n; ++i) {
        total += a[i];
    }
    return total;
}

double sum2(const double* a, std::size_t n) {
    double s0 = 0, s1 = 0;
    std::size_t i = 0;
    for (; i + 2 <= n; i += 2) {
        s0 += a[i];
        s1 += a[i + 1];
    }
    for (; i < n; ++i) {
        s0 += a[i];
    }
    return s0 + s1;
}

double sum4(const double* a, std::size_t n) {
    double s0 = 0, s1 = 0, s2 = 0, s3 = 0;
    std::size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        s0 += a[i];
        s1 += a[i + 1];
        s2 += a[i + 2];
        s3 += a[i + 3];
    }
    for (; i < n; ++i) {
        s0 += a[i];
    }
    return (s0 + s1) + (s2 + s3);
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
    // p(x) = even(x²) + x · odd(x²), where even has a[0], a[2], a[4], …
    // and odd has a[1], a[3], a[5], …. The two chains don't depend on
    // each other, so the CPU runs them side by side.
    double x2 = x * x;
    double even = 0, odd = 0;
    std::size_t i = n;
    if (i % 2 == 1) {
        --i;
        even = a[i];  // the top coefficient is even and has no odd partner
    }
    for (; i >= 2; i -= 2) {
        odd = a[i - 1] + x2 * odd;
        even = a[i - 2] + x2 * even;
    }
    return even + x * odd;
}
