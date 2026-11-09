#include "bench.hpp"

#include <algorithm>
#include <chrono>

double median(std::vector<double> xs) {
    // TODO
    (void)xs;
    return 0;
}

double ns_per_call(const std::function<void()>& f, int reps, int samples) {
    // TODO: time each batch with std::chrono::steady_clock.
    (void)f;
    (void)reps;
    (void)samples;
    return 0;
}
