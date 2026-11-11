// Brightens a made-up greyscale image, one call to brighten() per pixel.
// You won't need to change this file.
//
//   brighten [MB]     an image of MB megabytes (default 64)
//
// The checksum goes to stdout, the time to stderr.
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "pixel.hpp"

int main(int argc, char** argv) {
    std::size_t mb = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 64;
    std::vector<std::uint8_t> img(mb << 20);
    std::uint32_t seed = 12345;
    for (std::uint8_t& p : img) {
        seed = seed * 1664525 + 1013904223;
        p = static_cast<std::uint8_t>(seed >> 24);
    }

    auto start = std::chrono::steady_clock::now();
    for (std::uint8_t& p : img) {
        p = brighten(p, 40);
    }
    std::chrono::duration<double, std::milli> took = std::chrono::steady_clock::now() - start;

    std::uint32_t sum = 0;
    for (std::uint8_t p : img) {
        sum = sum * 31 + p;
    }
    std::printf("checksum %08x\n", sum);
    std::fprintf(stderr, "brightened %zu MB in %.1f ms\n", mb, took.count());
}
