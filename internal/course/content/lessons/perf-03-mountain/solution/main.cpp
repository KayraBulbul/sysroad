// Prints the memory mountain: read throughput for each working-set size and
// stride. You won't need to change this file.
//
//   mountain            16K to 64M, strides 1 to 12
//   mountain 512M       16K to 512M
//   mountain 64M csv    the same numbers as CSV, for a spreadsheet or a plot
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "bench.hpp"
#include "mountain.hpp"

int main(int argc, char** argv) {
    std::size_t biggest = argc > 1 ? std::strtoull(argv[1], nullptr, 10) << 20 : 64u << 20;
    bool csv = argc > 2 && std::strcmp(argv[2], "csv") == 0;
    if (biggest < (16u << 10)) {
        std::fprintf(stderr, "usage: mountain [largest size in MB] [csv]\n");
        return 2;
    }

    std::vector<std::size_t> sizes;
    for (std::size_t s = biggest; s >= (16u << 10); s /= 2) {
        sizes.push_back(s);
    }
    std::vector<std::size_t> strides;
    for (std::size_t s = 1; s <= 12; ++s) {
        strides.push_back(s);
    }

    std::vector<long> data(biggest / sizeof(long));
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<long>(i);  // touches every page now, not while timing
    }

    // The fastest of 3 samples, each at least 1 ms.
    Mountain m = build_mountain(data, sizes, strides, [](const std::function<void()>& f) {
        return measure(f, 3, 1e6).min;
    });

    std::printf(csv ? "size" : "GB/s  ");
    for (std::size_t s : m.strides) {
        std::printf(csv ? ",%s" : "%6s", ("s" + std::to_string(s)).c_str());
    }
    std::printf("\n");
    for (std::size_t i = 0; i < m.sizes.size() && i < m.gbps.size(); ++i) {
        std::printf(csv ? "%s" : "%-6s", human_size(m.sizes[i]).c_str());
        for (double g : m.gbps[i]) {
            std::printf(csv ? ",%.2f" : "%6.1f", g);
        }
        std::printf("\n");
    }
}
