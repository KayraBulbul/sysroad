#include "doctest.h"

#include <chrono>
#include <string>

#include "blockers.hpp"

TEST_CASE("lowercase of nothing, and of every byte") {
    std::string empty;
    lowercase(empty.data());
    CHECK(empty.empty());
    std::string all;
    for (int c = 1; c < 256; ++c) {
        all += static_cast<char>(c);
    }
    std::string want = all;
    for (char& c : want) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c + 32);
        }
    }
    lowercase(all.data());
    CHECK(all == want);
}

TEST_CASE("lowercase stops at the first NUL") {
    char s[] = "AB\0CD";
    lowercase(s);
    CHECK(s[0] == 'a');
    CHECK(s[1] == 'b');
    CHECK(s[3] == 'C');
    CHECK(s[4] == 'D');
}

TEST_CASE("invert twice gives back the original") {
    std::vector<std::uint8_t> px(1000);
    for (std::size_t i = 0; i < px.size(); ++i) {
        px[i] = static_cast<std::uint8_t>(i * 37);
    }
    std::vector<std::uint8_t> before = px;
    Image img{px.data(), px.size()};
    invert(img);
    for (std::size_t i = 0; i < px.size(); ++i) {
        CHECK(px[i] == 255 - before[i]);
    }
    invert(img);
    CHECK(px == before);
}

TEST_CASE("invert touches only size pixels") {
    std::vector<std::uint8_t> px{1, 2, 3, 4};
    Image img{px.data(), 2};
    invert(img);
    CHECK(px == std::vector<std::uint8_t>{254, 253, 3, 4});
    Image none{px.data(), 0};
    invert(none);
    CHECK(px == std::vector<std::uint8_t>{254, 253, 3, 4});
}

TEST_CASE("threshold at the edges") {
    std::vector<std::uint8_t> px{0, 1, 254, 255};
    std::vector<std::uint8_t> a = px, b = px, c;
    threshold(a, 0);
    CHECK(a == std::vector<std::uint8_t>{255, 255, 255, 255});
    threshold(b, 255);
    CHECK(b == std::vector<std::uint8_t>{0, 0, 0, 255});
    threshold(c, 7);
    CHECK(c.empty());
}

TEST_CASE("lowercase of 400,000 letters is quick") {
    std::string s(400'000, 'Z');
    auto start = std::chrono::steady_clock::now();
    lowercase(s.data());
    double took = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    CHECK(s.find_first_not_of('z') == std::string::npos);
    CHECK(took < 0.1);
}
