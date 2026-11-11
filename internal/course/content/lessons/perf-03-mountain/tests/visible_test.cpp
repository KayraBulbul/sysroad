#include "doctest.h"

#include <numeric>

#include "mountain.hpp"

TEST_CASE("read_stride") {
    std::vector<long> data(100);
    std::iota(data.begin(), data.end(), 0);  // 0, 1, 2, …, 99
    CHECK(read_stride(data.data(), 100, 1) == 4950);
    CHECK(read_stride(data.data(), 10, 3) == 0 + 3 + 6 + 9);
}

TEST_CASE("bytes_read") {
    CHECK(bytes_read(100, 1) == 800);
    CHECK(bytes_read(10, 3) == 32);  // reads 0, 3, 6 and 9
}

TEST_CASE("human_size") {
    CHECK(human_size(16384) == "16K");
    CHECK(human_size(1 << 20) == "1M");
    CHECK(human_size(100) == "100");
}

TEST_CASE("build_mountain fills a row per size and a column per stride") {
    std::vector<long> data(4096);
    // Pretends every call takes 1000 ns.
    Timer fake = [](const std::function<void()>& f) {
        f();
        return 1000.0;
    };
    Mountain m = build_mountain(data, {32768, 8192}, {1, 2, 8}, fake);
    REQUIRE(m.gbps.size() == 2);
    REQUIRE(m.gbps[0].size() == 3);
    CHECK(m.gbps[0][0] == doctest::Approx(32.768));  // 32768 bytes in 1000 ns
    CHECK(m.gbps[1][2] == doctest::Approx(1.024));   // 1024 longs, every 8th: 128 × 8 bytes
}
