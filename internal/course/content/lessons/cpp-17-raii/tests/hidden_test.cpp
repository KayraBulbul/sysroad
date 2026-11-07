#include "doctest.h"

#include <fcntl.h>
#include <fstream>
#include <string>
#include <type_traits>
#include <unistd.h>

#include "fd.hpp"

static bool is_open(int fd) {
    return ::fcntl(fd, F_GETFD) != -1;
}

static void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}

static_assert(!std::is_copy_constructible_v<Fd>);
static_assert(!std::is_copy_assignable_v<Fd>);

TEST_CASE("open_read: a missing file") {
    Fd fd = open_read("does-not-exist");
    CHECK_FALSE(fd.valid());
    CHECK(fd.get() == -1);
}

TEST_CASE("release hands the fd over without closing it") {
    write_file("r.txt", "x");
    int raw = -1;
    {
        Fd fd = open_read("r.txt");
        raw = fd.release();
        CHECK_FALSE(fd.valid());
    }
    CHECK(is_open(raw));
    ::close(raw);
}

TEST_CASE("reset closes the old fd") {
    write_file("s1.txt", "1");
    write_file("s2.txt", "2");
    Fd fd = open_read("s1.txt");
    int first = fd.get();
    int second = ::open("s2.txt", O_RDONLY);
    fd.reset(second);
    CHECK_FALSE(is_open(first));
    CHECK(fd.get() == second);
    CHECK(read_all(fd) == "2");
    fd.reset();
    CHECK_FALSE(is_open(second));
    CHECK_FALSE(fd.valid());
}

TEST_CASE("reset to the same fd keeps it open") {
    write_file("same.txt", "same");
    Fd fd = open_read("same.txt");
    fd.reset(fd.get());
    CHECK(is_open(fd.get()));
    CHECK(read_all(fd) == "same");
}

TEST_CASE("read_all: bigger than one read, with zero bytes") {
    std::string data(10000, 'a');
    data[5000] = '\0';
    write_file("big.bin", data);
    Fd fd = open_read("big.bin");
    CHECK(read_all(fd) == data);
}

TEST_CASE("read_all: an empty file") {
    write_file("empty.txt", "");
    Fd fd = open_read("empty.txt");
    CHECK(read_all(fd) == "");
}

TEST_CASE("no fds leak") {
    write_file("leak.txt", "x");
    for (int i = 0; i < 5000; ++i) {
        Fd fd = open_read("leak.txt");
        REQUIRE(fd.valid());
    }
    Fd last = open_read("leak.txt");
    CHECK(last.get() < 20);
}
