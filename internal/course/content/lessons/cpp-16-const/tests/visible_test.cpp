#include "doctest.h"

#include <string>

#include "inventory.hpp"

static int how_many(const Inventory& inv, const std::string& name) {
    return inv.count(name);
}

TEST_CASE("count through a const reference") {
    Inventory inv;
    inv.add("bolt", 5);
    inv.add("bolt", 2);
    CHECK(how_many(inv, "bolt") == 7);
}

TEST_CASE("asking for a missing item doesn't add it") {
    Inventory inv;
    inv.add("nut", 1);
    const Inventory& view = inv;
    CHECK(view.count("washer") == 0);
    CHECK(view.size() == 1);
}

static_assert(kib(4) == 4096, "kib must work at compile time");
