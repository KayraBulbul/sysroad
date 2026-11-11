#include "doctest.h"

#include <chrono>
#include <string>

#include "blockers.hpp"

TEST_CASE("lowercase") {
    std::string s = "Hello, World! ÄB";
    lowercase(s.data());
    CHECK(s == "hello, world! Äb");
}

TEST_CASE("invert") {
    std::vector<std::uint8_t> px{0, 1, 200, 255};
    Image img{px.data(), px.size()};
    invert(img);
    CHECK(px == std::vector<std::uint8_t>{255, 254, 55, 0});
}

TEST_CASE("threshold") {
    std::vector<std::uint8_t> px{0, 99, 100, 255};
    threshold(px, 100);
    CHECK(px == std::vector<std::uint8_t>{0, 0, 255, 255});
}

// The time to lowercase a string of n letters, fastest of 3, in seconds.
static double lowercase_seconds(std::size_t n) {
    double best = 1e9;
    for (int run = 0; run < 3; ++run) {
        std::string s(n, 'Q');
        auto start = std::chrono::steady_clock::now();
        lowercase(s.data());
        best = std::min(
            best, std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count());
    }
    return best;
}

TEST_CASE("lowercase takes time in proportion to the length") {
    double small = lowercase_seconds(20'000);
    double big = lowercase_seconds(200'000);
    CAPTURE(small);
    CAPTURE(big);
    // 10 times the letters should take about 10 times as long. 100 times as
    // long means the work grows with the square of the length.
    CHECK(big < 30 * small);
}
