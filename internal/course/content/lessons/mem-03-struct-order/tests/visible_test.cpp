#include "doctest.h"

#include <string>
#include <vector>

#include "pack.hpp"
#include "record.hpp"

static std::string names(const std::vector<Field>& fs) {
    std::string out;
    for (const Field& f : fs) {
        out += f.name + " ";
    }
    return out;
}

TEST_CASE("packed puts the largest alignment first") {
    std::vector<Field> fs = {{"a", 1, 1}, {"d", 8, 8}, {"b", 1, 1}, {"n", 4, 4}};
    CHECK(names(packed(fs)) == "d n a b ");
    CHECK(lay_out(packed(fs)).size == 16);
}

TEST_CASE("Record is 24 bytes") {
    CHECK(sizeof(Record) == 24);
}
