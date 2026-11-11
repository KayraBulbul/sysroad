#include "branches.hpp"

std::size_t filter_above(const int* in, std::size_t n, int limit, int* out) {
    // Always write, and only move on when the element was a keeper. The
    // next write overwrites the ones that weren't.
    std::size_t count = 0;
    for (std::size_t i = 0; i < n; ++i) {
        out[count] = in[i];
        count += in[i] > limit;
    }
    return count;
}

std::size_t lower_bound_index(const int* a, std::size_t n, int x) {
    // Keep a window [base, base + n) that holds the answer, and halve it
    // with a conditional move instead of a branch. Every search takes the
    // same number of steps.
    const int* base = a;
    while (n > 1) {
        std::size_t half = n / 2;
        base = __builtin_unpredictable(base[half - 1] < x) ? base + half : base;
        n -= half;
    }
    return static_cast<std::size_t>(base - a) + (n == 1 && *base < x);
}
