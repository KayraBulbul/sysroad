#include "mountain.hpp"

#include "bench.hpp"

long read_stride(const long* data, std::size_t elems, std::size_t stride) {
    // Four sums, so each add doesn't have to wait for the one before it.
    long a = 0, b = 0, c = 0, d = 0;
    std::size_t step = 4 * stride;
    std::size_t i = 0;
    for (; i + 3 * stride < elems; i += step) {
        a += data[i];
        b += data[i + stride];
        c += data[i + 2 * stride];
        d += data[i + 3 * stride];
    }
    for (; i < elems; i += stride) {
        a += data[i];
    }
    return a + b + c + d;
}

std::size_t bytes_read(std::size_t elems, std::size_t stride) {
    return (elems / stride + (elems % stride != 0)) * sizeof(long);
}

std::string human_size(std::size_t bytes) {
    const char* units[] = {"", "K", "M", "G", "T"};
    int u = 0;
    while (u < 4 && bytes >= 1024 && bytes % 1024 == 0) {
        bytes /= 1024;
        ++u;
    }
    return std::to_string(bytes) + units[u];
}

Mountain build_mountain(const std::vector<long>& data, const std::vector<std::size_t>& sizes,
                        const std::vector<std::size_t>& strides, const Timer& time) {
    Mountain m{sizes, strides, {}};
    for (std::size_t size : sizes) {
        std::size_t elems = size / sizeof(long);
        std::vector<double>& row = m.gbps.emplace_back();
        for (std::size_t stride : strides) {
            double ns = time([&] { do_not_optimize(read_stride(data.data(), elems, stride)); });
            // Bytes per nanosecond is GB per second.
            row.push_back(static_cast<double>(bytes_read(elems, stride)) / ns);
        }
    }
    return m;
}
