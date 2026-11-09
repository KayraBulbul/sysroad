// Calls each function with an input that hits its bug. You won't need to change this file.
#include <climits>
#include <cstdio>
#include <vector>

#include "ub.hpp"

int main() {
    std::printf("midpoint(INT_MAX, INT_MAX - 2) = %d\n", midpoint(INT_MAX, INT_MAX - 2));
    std::printf("rotl(0x12345678, 0)  = %#x\n", rotl(0x12345678, 0));
    std::printf("rotl(0x12345678, 8)  = %#x\n", rotl(0x12345678, 8));

    alignas(4) unsigned char bytes[8] = {0, 0x78, 0x56, 0x34, 0x12, 0, 0, 0};
    std::printf("read_u32(bytes + 1)  = %#x\n", read_u32(bytes + 1));

    std::vector<int> v = {1, 2, 3, 4};
    std::printf("has_adjacent_equal({1, 2, 3, 4}) = %s\n", has_adjacent_equal(v) ? "yes" : "no");

    std::printf("magnitude(INT_MIN)   = %u\n", magnitude(INT_MIN));
}
