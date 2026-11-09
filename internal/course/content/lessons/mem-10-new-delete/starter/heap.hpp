#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>

// A new int[n] holding n copies of value. The caller delete[]s it.
int* make_filled(std::size_t n, int value);

// Moves a new[]'d array of old_n ints into a new array of new_n ints: the old
// values first (as many as fit), then zeros. Frees the old array and returns
// the new one, which the caller delete[]s.
int* grow(int* old, std::size_t old_n, std::size_t new_n);

// How many times pushing n ints into an empty std::vector<int> calls new,
// optionally after v.reserve(n).
long vector_allocations(std::size_t n, bool reserve_first);
