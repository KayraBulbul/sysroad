#include "conv.hpp"

bool below(int x, unsigned limit) {
    return x < limit;
}

int last_index_of(const std::vector<int>& v, int x) {
    for (std::size_t i = v.size() - 1; i >= 0; --i) {
        if (v[i] == x) {
            return i;
        }
    }
    return -1;
}

std::array<int, 256> byte_histogram(std::string_view s) {
    std::array<int, 256> counts{};
    for (char c : s) {
        ++counts[c];
    }
    return counts;
}

std::uint8_t average(std::uint8_t a, std::uint8_t b) {
    std::uint8_t sum = a + b;
    return sum / 2;
}

std::uint64_t mib_to_bytes(std::uint32_t mib) {
    return mib * 1024 * 1024;
}

std::uint64_t bit(int n) {
    return 1 << n;
}
