#include "doctest.h"

#include <fcntl.h>
#include <fstream>
#include <string>
#include <unistd.h>

#include "fd.hpp"

static bool is_open(int fd) {
    return ::fcntl(fd, F_GETFD) != -1;
}

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

TEST_CASE("the destructor closes the fd") {
    write_file("a.txt", "hi");
    int raw = -1;
    {
        Fd fd = open_read("a.txt");
        REQUIRE(fd.valid());
        raw = fd.get();
        CHECK(is_open(raw));
    }
    CHECK_FALSE(is_open(raw));
}

TEST_CASE("read_all") {
    write_file("b.txt", "hello\nworld\n");
    Fd fd = open_read("b.txt");
    CHECK(read_all(fd) == "hello\nworld\n");
}
