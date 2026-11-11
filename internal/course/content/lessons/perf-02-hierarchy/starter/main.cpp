// Runs some access patterns through a simulated L1 cache. You won't need
// to change this file.
#include <cstdint>
#include <cstdio>
#include <functional>

#include "cache.hpp"

// A typical L1 data cache: 32 KB, 64-byte lines, 8 ways, so 64 sets.
static const CacheConfig l1{32 * 1024, 64, 8};

// Calls pattern with a function that reads one address, and prints how
// the cache did.
static void trace(const char* what,
                  const std::function<void(const std::function<void(std::uint64_t)>&)>& pattern) {
    Cache c(l1);
    pattern([&](std::uint64_t addr) { c.access(addr); });
    std::uint64_t total = c.hits() + c.misses();
    std::printf("%-40s %9llu %9llu %7.2f%%\n", what, (unsigned long long)total,
                (unsigned long long)c.misses(), total ? 100.0 * c.misses() / total : 0.0);
}

int main() {
    std::printf("%-40s %9s %9s %8s\n", "L1: 32 KB, 64 B lines, 8 ways", "reads", "misses", "rate");

    trace("1 MB of ints, in order", [](auto read) {
        for (std::uint64_t a = 0; a < (1 << 20); a += 4)
            read(a);
    });
    trace("1 MB of ints, every 16th (one per line)", [](auto read) {
        for (std::uint64_t a = 0; a < (1 << 20); a += 64)
            read(a);
    });
    trace("16 KB of ints, 10 times", [](auto read) {
        for (int pass = 0; pass < 10; ++pass)
            for (std::uint64_t a = 0; a < 16 * 1024; a += 4)
                read(a);
    });
    trace("64 KB of ints, 10 times", [](auto read) {
        for (int pass = 0; pass < 10; ++pass)
            for (std::uint64_t a = 0; a < 64 * 1024; a += 4)
                read(a);
    });

    // A 512 x 512 matrix of ints, one row after another in memory. A row is
    // 2048 bytes, so walking down a column jumps 2048 bytes per step.
    trace("512x512 ints, row by row", [](auto read) {
        for (std::uint64_t i = 0; i < 512; ++i)
            for (std::uint64_t j = 0; j < 512; ++j)
                read((i * 512 + j) * 4);
    });
    trace("512x512 ints, column by column", [](auto read) {
        for (std::uint64_t j = 0; j < 512; ++j)
            for (std::uint64_t i = 0; i < 512; ++i)
                read((i * 512 + j) * 4);
    });
    // The same, with each row padded by 16 unused ints (one line).
    trace("512x512 ints, rows padded to 528", [](auto read) {
        for (std::uint64_t j = 0; j < 512; ++j)
            for (std::uint64_t i = 0; i < 512; ++i)
                read((i * 528 + j) * 4);
    });
}
