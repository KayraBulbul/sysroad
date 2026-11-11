#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <functional>
#include <vector>

// Returns nanoseconds since some fixed point. The tests pass fake clocks,
// so they don't depend on how fast this machine is.
using Clock = std::function<double()>;

// The real clock: std::chrono::steady_clock, in nanoseconds.
double steady_ns();

// Time per call, in nanoseconds, over several samples.
struct Stats {
    double min = 0;
    double median = 0;
    double max = 0;
    long reps = 0;    // calls per sample
    int samples = 0;  // how many samples were taken

    // How far apart the samples are, relative to the median: 0.05 is 5%.
    double spread() const {
        return median > 0 ? (max - min) / median : 0;
    }
};

// calibrate never asks for more calls than this per batch.
constexpr long max_reps = 1L << 20;

// min, median and max of xs, and samples = how many there are. Leaves
// reps at 0. xs isn't empty.
Stats summarize(std::vector<double> xs);

// The first reps in 1, 2, 4, 8, … for which one batch of reps calls to f
// takes at least target_ns, timed with now. Stops at max_reps.
long calibrate(const std::function<void()>& f, double target_ns, const Clock& now = steady_ns);

// Picks reps with calibrate (which also warms f up), then times `samples`
// batches of reps calls and summarizes the time per call.
Stats measure(const std::function<void()>& f, int samples, double target_ns = 1e6,
              const Clock& now = steady_ns);

// Makes the compiler treat value as used, so it can't delete the work that
// produced it. Emits no instructions.
template <typename T> inline void do_not_optimize(const T& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}
