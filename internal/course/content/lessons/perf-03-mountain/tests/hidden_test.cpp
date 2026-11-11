#include "doctest.h"

#include <numeric>

#include "mountain.hpp"

TEST_CASE("read_stride with every count of leftovers") {
    std::vector<long> data(50);
    std::iota(data.begin(), data.end(), 1);  // 1, 2, …, 50
    for (std::size_t elems = 0; elems <= 50; ++elems) {
        for (std::size_t stride = 1; stride <= 9; ++stride) {
            long want = 0;
            for (std::size_t i = 0; i < elems; i += stride) {
                want += data[i];
            }
            CAPTURE(elems);
            CAPTURE(stride);
            CHECK(read_stride(data.data(), elems, stride) == want);
        }
    }
}

TEST_CASE("a stride past the end reads one element") {
    std::vector<long> data{7, 8, 9};
    CHECK(read_stride(data.data(), 3, 100) == 7);
    CHECK(bytes_read(3, 100) == 8);
}

TEST_CASE("bytes_read rounds the element count up") {
    CHECK(bytes_read(0, 1) == 0);
    CHECK(bytes_read(1, 1) == 8);
    CHECK(bytes_read(9, 4) == 24);  // 0, 4, 8
    CHECK(bytes_read(8, 4) == 16);  // 0, 4
    CHECK(bytes_read(1 << 20, 8) == (1 << 17) * 8);
}

TEST_CASE("human_size") {
    CHECK(human_size(0) == "0");
    CHECK(human_size(1024) == "1K");
    CHECK(human_size(1536) == "1536");
    CHECK(human_size(512 * 1024) == "512K");
    CHECK(human_size(3u << 20) == "3M");
    CHECK(human_size(std::size_t{1} << 30) == "1G");
    CHECK(human_size(std::size_t{2} << 40) == "2T");
    CHECK(human_size((1 << 20) + 1024) == "1025K");
}

TEST_CASE("build_mountain goes size by size, stride by stride") {
    std::vector<long> data(1024);  // exactly the largest size: 8 KB
    int calls = 0;
    // The first call takes 1000 ns, the second 2000, and so on.
    Timer fake = [&](const std::function<void()>& f) {
        f();
        return 1000.0 * ++calls;
    };
    Mountain m = build_mountain(data, {8192, 4096, 2048}, {1, 3}, fake);
    CHECK(calls == 6);
    CHECK(m.sizes == std::vector<std::size_t>{8192, 4096, 2048});
    CHECK(m.strides == std::vector<std::size_t>{1, 3});
    REQUIRE(m.gbps.size() == 3);
    for (const auto& row : m.gbps) {
        REQUIRE(row.size() == 2);
    }
    CHECK(m.gbps[0][0] == doctest::Approx(8192 / 1000.0));
    CHECK(m.gbps[0][1] == doctest::Approx(bytes_read(1024, 3) / 2000.0));
    CHECK(m.gbps[1][0] == doctest::Approx(4096 / 3000.0));
    CHECK(m.gbps[2][1] == doctest::Approx(bytes_read(256, 3) / 6000.0));
}
