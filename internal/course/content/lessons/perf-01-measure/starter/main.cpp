// Measures a few things with your harness. You won't need to change this file.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <list>
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
    std::printf("memset 256 MB: first %.1f ms, second %.1f ms\n\n", (t1 - t0) / 1e6,
                (t2 - t1) / 1e6);

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
    row("sum 4 KB, per int",
        measure([&] { do_not_optimize(std::accumulate(small.begin(), small.end(), 0)); }, 9),
        small.size());
    row("sum 64 MB, per int",
        measure([&] { do_not_optimize(std::accumulate(large.begin(), large.end(), 0)); }, 5, 2e7),
        large.size());

    // Two measurements promised back in phase 2. Your harness's numbers
    // are only as good as the code it times, so these print milliseconds.
    auto ms = [](const std::function<void()>& f) { return measure(f, 3).min / 1e6; };
    const int n = 1'000'000;
    std::vector<int> vec(n, 1);
    std::list<int> in_order(vec.begin(), vec.end());
    // The same values, but each node allocated in a random order relative
    // to the list order, as happens after a program has run a while.
    std::vector<std::list<int>::iterator> nodes;
    std::list<int> scattered;
    for (int i = 0; i < n; ++i) {
        nodes.push_back(scattered.insert(scattered.end(), 1));
    }
    std::uint32_t seed = 7;
    for (int i = n - 1; i > 0; --i) {
        seed = seed * 1664525 + 1013904223;
        std::swap(nodes[i], nodes[seed % static_cast<std::uint32_t>(i + 1)]);
    }
    std::list<int> shuffled;
    for (auto it : nodes) {
        shuffled.splice(shuffled.end(), scattered, it);
    }
    std::printf("\nsumming 1M ints (lesson 2.2.6):\n");
    std::printf("  %-28s %8.2f ms\n", "std::vector",
                ms([&] { do_not_optimize(std::accumulate(vec.begin(), vec.end(), 0)); }));
    std::printf("  %-28s %8.2f ms\n", "std::list, nodes in order",
                ms([&] { do_not_optimize(std::accumulate(in_order.begin(), in_order.end(), 0)); }));
    std::printf("  %-28s %8.2f ms\n", "std::list, nodes scattered",
                ms([&] { do_not_optimize(std::accumulate(shuffled.begin(), shuffled.end(), 0)); }));

    std::printf("\nbuilding 20,000 ints in reverse order (lesson 2.4.3):\n");
    std::printf("  %-28s %8.2f ms\n", "insert at begin()", ms([] {
                    std::vector<int> v;
                    for (int i = 0; i < 20'000; ++i) {
                        v.insert(v.begin(), i);
                    }
                    do_not_optimize(v.data());
                }));
    std::printf("  %-28s %8.2f ms\n", "push_back, then reverse", ms([] {
                    std::vector<int> v;
                    for (int i = 0; i < 20'000; ++i) {
                        v.push_back(i);
                    }
                    std::reverse(v.begin(), v.end());
                    do_not_optimize(v.data());
                }));
}
