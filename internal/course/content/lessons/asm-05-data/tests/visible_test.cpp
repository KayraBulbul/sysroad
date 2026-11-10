#include "doctest.h"

#include <vector>

#include "layout.hpp"
#include "mystery.hpp"

// One example each. Submit checks many more inputs.

TEST_CASE("kilo") {
    const long a[] = {10, 20, 30, 40};
    CHECK(kilo(a, 4) == 100);
}

TEST_CASE("lima") {
    const int a[] = {1, 2, 3, 4, 5};
    CHECK(lima(a, 1) == 6);
}

TEST_CASE("mike") {
    long a = 40;
    const long b = 2;
    mike(&a, &b);
    CHECK(a == 42);
}

TEST_CASE("Item is as big as the listing says") {
    CHECK(sizeof(Item) == 24);
}

TEST_CASE("november, on items built from bytes") {
    // id, shelf, tag, price, count
    auto items = items_from({{1, 2, 'a', 250, 4}, {2, 2, 'b', 100, 3}});
    CHECK(november(items.data(), 2) == 1300);
}

TEST_CASE("oscar, on an item built from bytes") {
    auto items = items_from({{100, 20, 'A', 999, 7}});
    CHECK(oscar(items.data()) == 185);
}

TEST_CASE("papa") {
    const Node c{30, 300, nullptr};
    const Node b{20, 200, &c};
    const Node a{10, 100, &b};
    CHECK(papa(&a, 20) == 200);
    CHECK(papa(&a, 99) == -1);
}
