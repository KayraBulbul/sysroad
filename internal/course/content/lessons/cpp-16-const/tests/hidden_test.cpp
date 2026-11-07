#include "doctest.h"

#include <array>
#include <string>
#include <type_traits>
#include <vector>

#include "inventory.hpp"

TEST_CASE("names are sorted, through a const object") {
    Inventory inv;
    inv.add("zip", 1);
    inv.add("axle", 2);
    inv.add("gear", 3);
    const Inventory& view = inv;
    CHECK(view.names() == std::vector<std::string>{"axle", "gear", "zip"});
    CHECK(view.size() == 3);
}

TEST_CASE("total adds every count") {
    Inventory inv;
    inv.add("a", 2);
    inv.add("b", 3);
    inv.add("a", 1);
    const Inventory& view = inv;
    CHECK(total(view) == 6);
    CHECK(total(Inventory{}) == 0);
}

TEST_CASE("total takes a const reference") {
    using Takes = int (*)(const Inventory&);
    Takes f = &total;  // only compiles if total takes const Inventory&
    Inventory inv;
    inv.add("x", 4);
    CHECK(f(inv) == 4);
}

TEST_CASE("a const Inventory can be fully read") {
    Inventory inv;
    inv.add("cog", 9);
    const Inventory copy = inv;
    CHECK(copy.count("cog") == 9);
    CHECK(copy.count("sprocket") == 0);
    CHECK(copy.size() == 1);
}

TEST_CASE("kib at compile time, as an array size") {
    std::array<char, kib(1)> buf{};
    CHECK(buf.size() == 1024);
    static_assert(kib(64) == 65536);
}
