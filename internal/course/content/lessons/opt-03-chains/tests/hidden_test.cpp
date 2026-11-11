#include "doctest.h"

#include <vector>

#include "chains.hpp"

static std::vector<double> counting(std::size_t n) {
    std::vector<double> a(n);
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<double>(i * 7 % 23) - 11;
    }
    return a;
}

TEST_CASE("sum2 and sum4 for every count of leftovers") {
    for (std::size_t n = 0; n <= 21; ++n) {
        std::vector<double> a = counting(n);
        CAPTURE(n);
        CHECK(sum2(a.data(), n) == sum1(a.data(), n));
        CHECK(sum4(a.data(), n) == sum1(a.data(), n));
    }
}

TEST_CASE("sum4 of a long array") {
    std::vector<double> a = counting(10'003);
    CHECK(sum4(a.data(), a.size()) == sum1(a.data(), a.size()));
    CHECK(sum2(a.data(), a.size()) == sum1(a.data(), a.size()));
}

TEST_CASE("sums read only the first n elements") {
    std::vector<double> a{1, 1, 1, 1, 1, 100, 100, 100, 100};
    CHECK(sum2(a.data(), 5) == 5);
    CHECK(sum4(a.data(), 5) == 5);
}

TEST_CASE("poly_split matches poly_horner for every length") {
    for (std::size_t n = 0; n <= 12; ++n) {
        std::vector<double> a = counting(n);
        for (double x : {0.0, 1.0, -1.0, 0.5, -1.25, 3.0}) {
            CAPTURE(n);
            CAPTURE(x);
            CHECK(poly_split(a.data(), n, x) == doctest::Approx(poly_horner(a.data(), n, x)));
        }
    }
}

TEST_CASE("poly_split with odd and even lengths") {
    std::vector<double> a{5};  // the constant 5
    CHECK(poly_split(a.data(), 1, 10) == 5);
    std::vector<double> b{1, 1, 1};  // 1 + x + x²
    CHECK(poly_split(b.data(), 3, 3) == 13);
    std::vector<double> c{0, 0, 0, 0, 1};  // x⁴
    CHECK(poly_split(c.data(), 5, 2) == 16);
}
