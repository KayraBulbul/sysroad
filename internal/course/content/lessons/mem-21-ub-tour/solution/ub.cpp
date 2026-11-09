#include "ub.hpp"

#include <cstring>

int midpoint(int a, int b) {
    return static_cast<int>((static_cast<long long>(a) + b) / 2);
}

std::uint32_t rotl(std::uint32_t x, unsigned n) {
    n %= 32;
    if (n == 0) {
        return x;
    }
    return (x << n) | (x >> (32 - n));
}

std::uint32_t read_u32(const unsigned char* p) {
    std::uint32_t v;
    std::memcpy(&v, p, sizeof v);
    return v;
}

bool has_adjacent_equal(const std::vector<int>& v) {
    for (std::size_t i = 0; i + 1 < v.size(); ++i) {
        if (v[i] == v[i + 1]) {
            return true;
        }
    }
    return false;
}

unsigned magnitude(int x) {
    return x < 0 ? 0u - static_cast<unsigned>(x) : static_cast<unsigned>(x);
}
