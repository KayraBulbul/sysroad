#include "ub.hpp"

#include <cstring>

int midpoint(int a, int b) {
    return (a + b) / 2;
}

std::uint32_t rotl(std::uint32_t x, unsigned n) {
    return (x << n) | (x >> (32 - n));
}

std::uint32_t read_u32(const unsigned char* p) {
    return *reinterpret_cast<const std::uint32_t*>(p);
}

bool has_adjacent_equal(const std::vector<int>& v) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (v[i] == v[i + 1]) {
            return true;
        }
    }
    return false;
}

unsigned magnitude(int x) {
    return x < 0 ? -x : x;
}
