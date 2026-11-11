#include "bench.hpp"

#include <algorithm>
#include <chrono>

double steady_ns() {
    return std::chrono::duration<double, std::nano>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

Stats summarize(std::vector<double> xs) {
    // TODO
    (void)xs;
    return {};
}

long calibrate(const std::function<void()>& f, double target_ns, const Clock& now) {
    // TODO: time batches of 1, 2, 4, … calls with now().
    (void)f;
    (void)target_ns;
    (void)now;
    return 1;
}

Stats measure(const std::function<void()>& f, int samples, double target_ns, const Clock& now) {
    // TODO
    (void)f;
    (void)samples;
    (void)target_ns;
    (void)now;
    return {};
}
