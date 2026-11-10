// Calls each function with an input that hits its bug. You won't need to change this file.
#include <cstdio>
#include <vector>

#include "conv.hpp"

int main() {
    std::printf("below(-1, 5)          = %s\n", below(-1, 5) ? "true" : "false");
    std::printf("average(200, 100)     = %d\n", average(200, 100));
    std::printf("mib_to_bytes(8192)    = %llu\n",
                static_cast<unsigned long long>(mib_to_bytes(8192)));
    std::printf("bit(31)               = %#llx\n", static_cast<unsigned long long>(bit(31)));
    std::printf("bit(40)               = %#llx\n", static_cast<unsigned long long>(bit(40)));
    std::printf("histogram of \"caf\\xc3\\xa9\" has %d of byte 0xc3\n",
                byte_histogram("caf\xc3\xa9")[0xc3]);
    std::printf("last_index_of({1, 2}, 7) = %d\n", last_index_of({1, 2}, 7));
}
