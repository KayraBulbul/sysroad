#include "doctest.h"

#include "archive.hpp"
#include "fixtures.hpp"

TEST_CASE("a GNU archive of text files") {
    Archive a = parse_archive(bytes_of(gnu_text));
    REQUIRE(a.members.size() == 3);
    CHECK(a.members[0].name == "a.txt");
    CHECK(a.members[0].data == "hello\n");
    CHECK(a.members[2].name == "odd.txt");
    CHECK(a.members[2].data == "odd");
    CHECK(a.symbols.empty());
}

TEST_CASE("a GNU long name") {
    Archive a = parse_archive(bytes_of(gnu_text));
    REQUIRE(a.members.size() == 3);
    CHECK(a.members[1].name == "a_member_with_a_long_name.txt");
    CHECK(a.members[1].data == "a much longer member name\n");
}

TEST_CASE("a GNU symbol table") {
    Archive a = parse_archive(bytes_of(gnu_objects));
    REQUIRE(a.members.size() == 2);
    CHECK(a.members[0].name == "add.o");
    CHECK(a.members[1].name == "multiply_with_a_long_file_name.o");
    REQUIRE(a.symbols.size() == 3);
    CHECK(a.symbols[0] == std::pair<std::string, std::string>{"add", "add.o"});
    CHECK(a.symbols[2] ==
          std::pair<std::string, std::string>{"mul", "multiply_with_a_long_file_name.o"});
}

TEST_CASE("a BSD archive, as on the Mac") {
    Archive a = parse_archive(bytes_of(bsd_objects));
    REQUIRE(a.members.size() == 2);
    CHECK(a.members[0].name == "add.o");
    CHECK(a.members[0].data.size() == 728);
    REQUIRE(a.symbols.size() == 3);
    CHECK(a.symbols[1] == std::pair<std::string, std::string>{"add_three", "add.o"});
}
