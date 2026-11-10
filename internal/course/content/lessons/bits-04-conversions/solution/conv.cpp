#include "conv.hpp"

bool below(int x, unsigned limit) {
    return x < 0 || static_cast<unsigned>(x) < limit;
}

int last_index_of(const std::vector<int>& v, int x) {
    for (std::size_t i = v.size(); i-- > 0;) {
        if (v[i] == x) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::array<int, 256> byte_histogram(std::string_view s) {
    std::array<int, 256> counts{};
    for (char c : s) {
        ++counts[static_cast<unsigned char>(c)];
    }
    return counts;
}

std::uint8_t average(std::uint8_t a, std::uint8_t b) {
    return static_cast<std::uint8_t>((a + b) / 2);
}

std::uint64_t mib_to_bytes(std::uint32_t mib) {
    return std::uint64_t{mib} * 1024 * 1024;
}

std::uint64_t bit(int n) {
    return std::uint64_t{1} << n;
}
