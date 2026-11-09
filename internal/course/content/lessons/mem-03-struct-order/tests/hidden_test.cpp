#include "doctest.h"

#include <cstdint>
#include <string>
#include <type_traits>
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

TEST_CASE("equal alignments keep their order") {
    std::vector<Field> fs = {{"c1", 1, 1}, {"s1", 2, 2}, {"c2", 1, 1},
                             {"s2", 2, 2}, {"c3", 1, 1}, {"arr", 12, 4}};
    CHECK(names(packed(fs)) == "arr s1 s2 c1 c2 c3 ");
    Layout l = lay_out(packed(fs));
    CHECK(l.size == 20);
    CHECK(l.tail_padding == 1);
}

TEST_CASE("packed doesn't lose or change fields") {
    std::vector<Field> fs = {{"x", 3, 1}, {"y", 16, 8}};
    std::vector<Field> p = packed(fs);
    REQUIRE(p.size() == 2);
    CHECK(p[0].name == "y");
    CHECK(p[0].size == 16);
    CHECK(p[1].size == 3);
    CHECK(packed({}).empty());
}

TEST_CASE("Record keeps every field") {
    CHECK(std::is_same_v<decltype(Record::active), bool>);
    CHECK(std::is_same_v<decltype(Record::score), double>);
    CHECK(std::is_same_v<decltype(Record::grade), char>);
    CHECK(std::is_same_v<decltype(Record::id), std::int64_t>);
    CHECK(std::is_same_v<decltype(Record::year), std::int16_t>);
    CHECK(std::is_same_v<decltype(Record::weight), float>);
    Record r{};
    r.id = 42;
    r.grade = 'A';
    CHECK(r.id == 42);
    CHECK(sizeof(Record) == 24);
}
