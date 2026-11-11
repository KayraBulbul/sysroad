#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstddef>

// Copies the elements of in[0, n) that are greater than limit to out, in
// order, and returns how many it copied. out has room for n.
std::size_t filter_above(const int* in, std::size_t n, int limit, int* out);

// The index of the first element of the sorted a[0, n) that's not less
// than x, or n if there's none. The same answer as std::lower_bound.
std::size_t lower_bound_index(const int* a, std::size_t n, int x);
