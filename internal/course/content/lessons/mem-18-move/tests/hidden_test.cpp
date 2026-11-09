#include "doctest.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "fd.hpp"
#include "fd_test_util.hpp"

static_assert(std::is_nothrow_move_constructible_v<Fd>);
static_assert(std::is_nothrow_move_assignable_v<Fd>);
static_assert(!std::is_copy_constructible_v<Fd>);

TEST_CASE("self-move keeps the fd") {
    write_file("s.txt", "self");
    Fd a = open_read("s.txt");
    int raw = a.get();
    Fd& same = a;
    a = std::move(same);
    CHECK(a.get() == raw);
    CHECK(is_open(raw));
    CHECK(read_all(a) == "self");
}

TEST_CASE("moving an empty Fd") {
    Fd empty;
    Fd b(std::move(empty));
    CHECK_FALSE(b.valid());
    write_file("e.txt", "x");
    Fd c = open_read("e.txt");
    int raw = c.get();
    c = Fd();  // assigning an empty one closes
    CHECK_FALSE(c.valid());
    CHECK_FALSE(is_open(raw));
}

TEST_CASE("a vector of 50 Fds opens and closes them all") {
    std::vector<int> raws;
    {
        std::vector<Fd> fds;  // no reserve: growing moves them
        for (int i = 0; i < 50; ++i) {
            std::string name = "v" + std::to_string(i) + ".txt";
            write_file(name, std::to_string(i));
            fds.push_back(open_read(name.c_str()));
            raws.push_back(fds.back().get());
        }
        for (int i = 0; i < 50; ++i) {
            CHECK(fds[i].get() == raws[i]);
        }
        CHECK(read_all(fds[42]) == "42");
    }
    for (int raw : raws) {
        CHECK_FALSE(is_open(raw));
    }
}

TEST_CASE("open_first") {
    write_file("f2.txt", "second");
    write_file("f3.txt", "third");
    Fd fd = open_first({"missing1", "f2.txt", "f3.txt"});
    REQUIRE(fd.valid());
    CHECK(read_all(fd) == "second");
    CHECK_FALSE(open_first({"missing1", "missing2"}).valid());
    CHECK_FALSE(open_first({}).valid());
}

TEST_CASE("open_all with nothing to open") {
    std::vector<std::string> failed = {"earlier"};
    CHECK(open_all({"a-missing", "b-missing"}, failed).empty());
    CHECK(failed == std::vector<std::string>{"earlier", "a-missing", "b-missing"});
}

TEST_CASE("open_first doesn't leak the ones it skips") {
    write_file("only.txt", "x");
    Fd probe = open_read("only.txt");
    int next_free = probe.get();
    probe.reset();
    Fd fd = open_first({"only.txt"});
    CHECK(fd.get() == next_free);  // the lowest free number: nothing else is open
}
