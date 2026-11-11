#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstddef>

// a[0] + a[1] + … + a[n - 1], one add after another. Already written.
double sum1(const double* a, std::size_t n);
// The same sum, with two running totals.
double sum2(const double* a, std::size_t n);
// The same sum, with four running totals.
double sum4(const double* a, std::size_t n);

// a[0] + a[1]·x + a[2]·x² + … + a[n - 1]·xⁿ⁻¹, by Horner's rule. Already written.
double poly_horner(const double* a, std::size_t n, double x);
// The same polynomial, as two Horner chains in x²: one for the even
// coefficients and one for the odd ones.
double poly_split(const double* a, std::size_t n, double x);
