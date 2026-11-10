#include "doctest.h"

#include <cstdint>
#include <vector>

#include "layout.hpp"
#include "mystery.hpp"

static std::uint64_t next(std::uint64_t& state) {
    state = state * 6364136223846793005u + 1442695040888963407u;
    return state >> 33;
}

TEST_CASE("kilo adds up n values") {
    std::uint64_t state = 21;
    std::vector<long> a(200);
    for (long& x : a) {
        x = static_cast<long>(next(state) % 2001) - 1000;
    }
    long sum = 0;
    for (std::size_t n = 0; n <= a.size(); ++n) {
        CAPTURE(n);
        REQUIRE(kilo(a.data(), static_cast<long>(n)) == sum);
        if (n < a.size()) {
            sum += a[n];
        }
    }
    CHECK(kilo(a.data(), 0) == 0);
    CHECK(kilo(a.data(), -3) == 0);
    CHECK(kilo(a.data() + 7, 1) == a[7]);
}

TEST_CASE("lima adds a[i] and a[i + 2]") {
    const int a[] = {5, -7, 100, 3000, -20000, 1, 9};
    for (long i = 0; i + 2 < 7; ++i) {
        CAPTURE(i);
        REQUIRE(lima(a, i) == a[i] + a[i + 2]);
    }
    CHECK(lima(a + 4, -2) == a[2] + a[4]);
}

TEST_CASE("mike adds *b into *a and leaves b alone") {
    long a = -5;
    long b = 12;
    mike(&a, &b);
    CHECK(a == 7);
    CHECK(b == 12);
    mike(&a, &b);
    CHECK(a == 19);
    long x = 21;
    mike(&x, &x);
    CHECK(x == 42);
}

TEST_CASE("november over many items") {
    REQUIRE(sizeof(Item) == 24);
    std::uint64_t state = 22;
    std::vector<Raw> raws;
    long total = 0;
    for (int i = 0; i < 300; ++i) {
        Raw r{};
        r.id = static_cast<int>(next(state) % 100000);
        r.shelf = static_cast<short>(next(state) % 30000);
        r.tag = static_cast<char>('a' + next(state) % 26);
        r.price = static_cast<long>(next(state) % 100000) - 5000;
        r.count = static_cast<int>(next(state) % 500) - 20;
        raws.push_back(r);
        auto items = items_from(raws);
        total += r.price * r.count;
        CAPTURE(i);
        REQUIRE(november(items.data(), static_cast<long>(items.size())) == total);
    }
    auto items = items_from(raws);
    CHECK(november(items.data(), 0) == 0);
    CHECK(november(items.data(), -1) == 0);
    CHECK(november(items.data() + 5, 1) == raws[5].price * raws[5].count);
}

TEST_CASE("november uses 64-bit prices") {
    REQUIRE(sizeof(Item) == 24);
    auto items = items_from({{1, 1, 'x', 5000000000, 3}, {2, 1, 'y', -7000000000, 2}});
    CHECK(november(items.data(), 2) == 1000000000);
}

TEST_CASE("oscar adds the three small fields, with their signs") {
    REQUIRE(sizeof(Item) == 24);
    auto items = items_from({
        {7, 3, 'B', 123456, 99},
        {-1000, -300, 'z', -1, -1},
        {2000000000, 32767, '\x7f', 0, 0},
        {0, 0, '\0', 77, 88},
        {5, -1, static_cast<char>(-2), 1, 1},
    });
    CHECK(oscar(&items[0]) == 7 + 3 + 'B');
    CHECK(oscar(&items[1]) == -1000 - 300 + 'z');
    CHECK(oscar(&items[2]) == 2000000000 + 32767 + 127);
    CHECK(oscar(&items[3]) == 0);
    CHECK(oscar(&items[4]) == 2);
}

TEST_CASE("papa walks the whole list") {
    std::vector<Node> nodes(50);
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].key = static_cast<long>(i * 3);
        nodes[i].value = static_cast<long>(i * i) - 7;
        nodes[i].next = i + 1 < nodes.size() ? &nodes[i + 1] : nullptr;
    }
    for (std::size_t i = 0; i < nodes.size(); ++i) {
        CAPTURE(i);
        REQUIRE(papa(nodes.data(), static_cast<long>(i * 3)) == nodes[i].value);
        REQUIRE(papa(nodes.data(), static_cast<long>(i * 3 + 1)) == -1);
    }
    CHECK(papa(nullptr, 0) == -1);
    // The first of two nodes with the same key wins.
    nodes[10].key = 0;
    CHECK(papa(nodes.data(), 0) == nodes[0].value);
    CHECK(papa(&nodes[5], 0) == nodes[10].value);
    // A search that starts in the middle doesn't see earlier nodes.
    CHECK(papa(&nodes[20], 3) == -1);
}
