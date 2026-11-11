// Races the branchy versions against yours, on data the branch predictor
// can and can't guess. You won't need to change this file.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <vector>

#include "bench.hpp"
#include "branches.hpp"

namespace original {
std::size_t filter_above(const int* in, std::size_t n, int limit, int* out) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (in[i] > limit) {
            out[count++] = in[i];
        }
    }
    return count;
}
std::size_t lower_bound_index(const int* a, std::size_t n, int x) {
    std::size_t lo = 0, hi = n;
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (a[mid] < x) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}
}  // namespace original

// xorshift: the same random numbers on every machine.
static std::uint32_t next_random() {
    static std::uint32_t s = 2463534242u;
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    return s;
}

static double best_ms(const std::function<void()>& f) {
    return measure(f, 5, 1e6).min / 1e6;
}

static void race(const char* what, double before, double after, bool same) {
    if (!same) {
        std::printf("%-34s %8.2f ms   (yours gives a different answer)\n", what, before);
        return;
    }
    std::printf("%-34s %8.2f ms %8.2f ms   %.1fx\n", what, before, after, before / after);
}

int main() {
    std::printf("%-34s %11s %11s\n", "", "branchy", "yours");

    const std::size_t n = 1 << 20;
    std::vector<int> shuffled(n);
    for (int& x : shuffled) {
        x = static_cast<int>(next_random() % 256);
    }
    std::vector<int> sorted = shuffled;
    std::sort(sorted.begin(), sorted.end());
    std::vector<int> out1(n), out2(n);

    for (const auto& [what, data] :
         {std::pair{"filter 1M, sorted", &sorted}, std::pair{"filter 1M, shuffled", &shuffled}}) {
        const int* in = data->data();
        std::size_t k1 = 0, k2 = 0;
        double before = best_ms([&] { k1 = original::filter_above(in, n, 127, out1.data()); });
        double after = best_ms([&] { k2 = filter_above(in, n, 127, out2.data()); });
        race(what, before, after,
             k1 == k2 && std::equal(out1.begin(), out1.begin() + k1, out2.begin()));
    }

    std::vector<int> table(1 << 16);
    for (std::size_t i = 0; i < table.size(); ++i) {
        table[i] = static_cast<int>(2 * i);
    }
    std::vector<int> queries(1 << 20);
    for (int& q : queries) {
        q = static_cast<int>(next_random() % (2 << 16));
    }
    std::size_t s1 = 0, s2 = 0;
    double before = best_ms([&] {
        s1 = 0;
        for (int q : queries) {
            s1 += original::lower_bound_index(table.data(), table.size(), q);
        }
    });
    double after = best_ms([&] {
        s2 = 0;
        for (int q : queries) {
            s2 += lower_bound_index(table.data(), table.size(), q);
        }
    });
    race("1M searches of 64K ints", before, after, s1 == s2);
}
