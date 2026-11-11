// Measures a few things with your harness. You won't need to change this file.
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <numeric>
#include <vector>

#include "bench.hpp"

// The smallest step steady_ns() takes between two readings.
static double clock_step() {
    double best = 1e18;
    for (int i = 0; i < 1000; ++i) {
        double a = steady_ns();
        double b = steady_ns();
        while (b == a) {
            b = steady_ns();
        }
        if (b - a < best) {
            best = b - a;
        }
    }
    return best;
}

static void row(const char* what, const Stats& s, double per = 1) {
    std::printf("%-26s %9.2f %9.2f %9.2f %7.1f%% %9ld\n", what, s.min / per, s.median / per,
                s.max / per, s.spread() * 100, s.reps);
}

int main() {
    std::printf("clock step: %.0f ns\n\n", clock_step());

    // The first pass over freshly allocated memory pays for the OS mapping
    // every page. The second doesn't.
    const std::size_t big = 256u << 20;
    std::unique_ptr<char[]> buf(new char[big]);
    double t0 = steady_ns();
    std::memset(buf.get(), 1, big);
    double t1 = steady_ns();
    std::memset(buf.get(), 2, big);
    double t2 = steady_ns();
    do_not_optimize(buf[big - 1]);
    std::printf("memset 256 MB: first %.1f ms, second %.1f ms\n\n", (t1 - t0) / 1e6, (t2 - t1) / 1e6);

    std::printf("%-26s %9s %9s %9s %8s %9s\n", "ns per call", "min", "median", "max", "spread",
                "reps");
    row("empty function", measure([] {}, 9));

    double x = 2;
    row("std::sqrt", measure(
                         [&] {
                             x = std::sqrt(x + 1);
                             do_not_optimize(x);
                         },
                         9));

    // The same loop over 4 KB and over 64 MB, per element.
    std::vector<int> small(1024, 1);
    std::vector<int> large(16u << 20, 1);
    row("sum 4 KB, per int", measure(
                                 [&] {
                                     do_not_optimize(std::accumulate(small.begin(), small.end(), 0));
                                 },
                                 9),
        small.size());
    row("sum 64 MB, per int", measure(
                                  [&] {
                                      do_not_optimize(std::accumulate(large.begin(), large.end(), 0));
                                  },
                                  5, 2e7),
        large.size());
}
