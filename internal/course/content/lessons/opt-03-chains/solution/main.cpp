// Measures cycles per element for each function. You won't need to change
// this file.
#include <cstdio>
#include <functional>
#include <numeric>
#include <vector>

#include "bench.hpp"
#include "chains.hpp"

// How long one integer add takes, in nanoseconds: a chain of adds, each
// waiting for the last. An add takes one cycle, so this is the length of
// a cycle.
static double cycle_ns() {
    const long n = 1 << 22;
    return measure(
               [&] {
                   long x = 0;
                   for (long i = 0; i < n; ++i) {
                       x += 1;
                       asm volatile("" : "+r"(x));  // stops clang from adding n in one go
                   }
                   do_not_optimize(x);
               },
               9)
               .min /
           n;
}

int main() {
    // A CPU that has been idle runs slowly for a few milliseconds before it
    // speeds up. Get that over with first.
    double start = steady_ns();
    while (steady_ns() - start < 200e6) {
        cycle_ns();
    }
    double cycle = cycle_ns();
    std::printf("one cycle is %.3f ns, so this core runs at about %.1f GHz\n\n", cycle, 1 / cycle);

    // 4096 doubles is 32 KB, small enough to stay in L1. The values are
    // whole numbers, so every way of adding them up gives the same answer.
    const std::size_t n = 4096;
    std::vector<double> a(n);
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = static_cast<double>(i % 10);
    }
    double want = sum1(a.data(), n);

    auto row = [&](const char* what, const std::function<double()>& f, double expect) {
        double got = f();
        Stats s = measure([&] { do_not_optimize(f()); }, 7);
        cycle = cycle_ns();  // again, in case the clock speed changed
        bool close = got - expect < 1e-6 * expect && expect - got < 1e-6 * expect;
        std::printf("%-26s %6.2f cycles per element%s\n", what, s.min / n / cycle,
                    close ? "" : "   (wrong answer)");
    };
    row("sum1", [&] { return sum1(a.data(), n); }, want);
    row("sum2", [&] { return sum2(a.data(), n); }, want);
    row("sum4", [&] { return sum4(a.data(), n); }, want);
    row("std::accumulate", [&] { return std::accumulate(a.begin(), a.end(), 0.0); }, want);

    double x = 0.999;
    double p = poly_horner(a.data(), n, x);
    row("poly_horner", [&] { return poly_horner(a.data(), n, x); }, p);
    row("poly_split", [&] { return poly_split(a.data(), n, x); }, p);
}
