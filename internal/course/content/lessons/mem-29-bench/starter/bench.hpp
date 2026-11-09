#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <functional>
#include <vector>

// The middle value of xs, or the mean of the two middle values when there's
// an even number of them. xs isn't empty.
double median(std::vector<double> xs);

// Calls f in `samples` batches of `reps` calls each, and returns the median
// time per call, in nanoseconds.
double ns_per_call(const std::function<void()>& f, int reps, int samples);

// Makes the compiler treat value as used, so it can't delete the work that
// produced it. Emits no instructions.
template <typename T> inline void do_not_optimize(const T& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}
