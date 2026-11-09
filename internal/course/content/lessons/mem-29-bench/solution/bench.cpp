#include "bench.hpp"

#include <algorithm>
#include <chrono>

double median(std::vector<double> xs) {
    std::sort(xs.begin(), xs.end());
    std::size_t mid = xs.size() / 2;
    return xs.size() % 2 == 1 ? xs[mid] : (xs[mid - 1] + xs[mid]) / 2;
}

double ns_per_call(const std::function<void()>& f, int reps, int samples) {
    std::vector<double> per_call;
    per_call.reserve(samples);
    for (int s = 0; s < samples; ++s) {
        auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < reps; ++i) {
            f();
        }
        std::chrono::duration<double, std::nano> took = std::chrono::steady_clock::now() - start;
        per_call.push_back(took.count() / reps);
    }
    return median(per_call);
}
