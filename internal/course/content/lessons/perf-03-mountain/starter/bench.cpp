// The course's version of lesson 5.1.1. You won't need to change this file.
#include "bench.hpp"

#include <algorithm>
#include <chrono>

double steady_ns() {
    return std::chrono::duration<double, std::nano>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

Stats summarize(std::vector<double> xs) {
    std::sort(xs.begin(), xs.end());
    std::size_t mid = xs.size() / 2;
    Stats s;
    s.min = xs.front();
    s.max = xs.back();
    s.median = xs.size() % 2 == 1 ? xs[mid] : (xs[mid - 1] + xs[mid]) / 2;
    s.samples = static_cast<int>(xs.size());
    return s;
}

// How long one batch of reps calls takes.
static double time_batch(const std::function<void()>& f, long reps, const Clock& now) {
    double start = now();
    for (long i = 0; i < reps; ++i) {
        f();
    }
    return now() - start;
}

long calibrate(const std::function<void()>& f, double target_ns, const Clock& now) {
    long reps = 1;
    while (reps < max_reps && time_batch(f, reps, now) < target_ns) {
        reps *= 2;
    }
    return reps;
}

Stats measure(const std::function<void()>& f, int samples, double target_ns, const Clock& now) {
    long reps = calibrate(f, target_ns, now);
    std::vector<double> per_call;
    per_call.reserve(samples);
    for (int s = 0; s < samples; ++s) {
        per_call.push_back(time_batch(f, reps, now) / static_cast<double>(reps));
    }
    Stats st = summarize(per_call);
    st.reps = reps;
    return st;
}
