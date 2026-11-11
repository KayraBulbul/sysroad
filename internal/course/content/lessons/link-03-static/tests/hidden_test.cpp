#include "doctest.h"

#include <stdexcept>
#include <string>

#include "archive.hpp"
#include "fixtures.hpp"

TEST_CASE("BSD text archive: names and data") {
    Archive a = parse_archive(bytes_of(bsd_text));
    REQUIRE(a.members.size() == 3);
    CHECK(a.members[0].name == "a.txt");
    CHECK(a.members[0].data == "hello\n");
    CHECK(a.members[1].name == "a_member_with_a_long_name.txt");
    CHECK(a.members[1].data == "a much longer member name\n");
    CHECK(a.members[2].name == "odd.txt");
    CHECK(a.members[2].data == "odd");
    CHECK(a.symbols.empty());
}

TEST_CASE("both formats give the same objects") {
    Archive g = parse_archive(bytes_of(gnu_objects));
    Archive b = parse_archive(bytes_of(bsd_objects));
    REQUIRE(g.members.size() == b.members.size());
    for (std::size_t i = 0; i < g.members.size(); ++i) {
        CHECK(g.members[i].name == b.members[i].name);
        CHECK(g.members[i].data == b.members[i].data);
        CHECK(g.members[i].data.starts_with("\x7f"
                                            "ELF"));
    }
    CHECK(g.symbols == b.symbols);
}

TEST_CASE("header offsets") {
    Archive a = parse_archive(bytes_of(gnu_text));
    REQUIRE(a.members.size() == 3);
    std::string_view all = bytes_of(gnu_text);
    for (const Member& m : a.members) {
        CHECK(all.substr(m.header_offset + 58, 2) == "`\n");
        CHECK(all.data() + m.header_offset + 60 == m.data.data());
    }
    // a.txt comes after the 8-byte magic and the long-name table.
    CHECK(a.members[0].header_offset == 8 + 60 + 32);
}

TEST_CASE("an empty archive") {
    Archive a = parse_archive("!<arch>\n");
    CHECK(a.members.empty());
    CHECK(a.symbols.empty());
}

TEST_CASE("bad archives throw") {
    CHECK_THROWS_AS(parse_archive(""), std::runtime_error);
    CHECK_THROWS_AS(parse_archive("!<arch>"), std::runtime_error);
    CHECK_THROWS_AS(parse_archive("\x7f"
                                  "ELF and so on"),
                    std::runtime_error);
    std::string_view whole = bytes_of(gnu_text);
    // Cut off in the middle of a header, and in the middle of a member.
    CHECK_THROWS_AS(parse_archive(whole.substr(0, 8 + 30)), std::runtime_error);
    CHECK_THROWS_AS(parse_archive(whole.substr(0, whole.size() - 2)), std::runtime_error);
    // odd.txt is the last member, 3 bytes and a padding '\n'. Without it:
    CHECK_THROWS_AS(parse_archive(whole.substr(0, whole.size() - 1)), std::runtime_error);
    // A header that doesn't end in `\n.
    std::string broken(whole);
    broken[8 + 58] = 'x';
    CHECK_THROWS_AS(parse_archive(broken), std::runtime_error);
}
