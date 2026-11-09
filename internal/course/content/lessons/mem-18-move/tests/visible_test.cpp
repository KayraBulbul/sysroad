#include "doctest.h"

#include <utility>

#include "fd.hpp"
#include "fd_test_util.hpp"

TEST_CASE("the move constructor takes the fd") {
    write_file("m1.txt", "x");
    Fd a = open_read("m1.txt");
    int raw = a.get();
    Fd b(std::move(a));
    CHECK(b.get() == raw);
    CHECK_FALSE(a.valid());
    CHECK(is_open(raw));
}

TEST_CASE("move assignment closes the old fd") {
    write_file("m2.txt", "x");
    write_file("m3.txt", "y");
    Fd a = open_read("m2.txt");
    Fd b = open_read("m3.txt");
    int old_b = b.get(), raw_a = a.get();
    b = std::move(a);
    CHECK(b.get() == raw_a);
    CHECK_FALSE(a.valid());
    CHECK_FALSE(is_open(old_b));
    CHECK(read_all(b) == "x");
}

TEST_CASE("open_all") {
    write_file("o1.txt", "one");
    write_file("o2.txt", "two");
    std::vector<std::string> failed;
    std::vector<Fd> fds = open_all({"o1.txt", "nope.txt", "o2.txt"}, failed);
    REQUIRE(fds.size() == 2);
    CHECK(read_all(fds[0]) == "one");
    CHECK(read_all(fds[1]) == "two");
    CHECK(failed == std::vector<std::string>{"nope.txt"});
}
