#include "doctest.h"

#include <cstdio>
#include <fstream>
#include <istream>
#include <iterator>
#include <string>
#include <vector>

#include "coreutils/io.hpp"

TEST_CASE("for_each_input reads each file and carries on past a bad one") {
    const char* path = "sysroad_for_each_input.txt";
    {
        std::ofstream out(path);
        out << "data\n";
    }
    std::vector<std::string> got;
    char good[] = "sysroad_for_each_input.txt";
    char bad[] = "no-such-file-2";
    char* files[] = {good, bad, good};
    int status =
        coreutils::for_each_input("t", files, [&](std::istream& in, std::string_view name) {
            got.push_back(std::string(name) + "=" +
                          std::string(std::istreambuf_iterator<char>(in), {}));
        });
    std::remove(path);
    CHECK(status == 1);
    CHECK(got == std::vector<std::string>{"sysroad_for_each_input.txt=data\n",
                                          "sysroad_for_each_input.txt=data\n"});
}

TEST_CASE("args skips the program name") {
    char p[] = "prog";
    char x[] = "x";
    char* argv[] = {p, x, nullptr};
    auto a = coreutils::args(2, argv);
    REQUIRE(a.size() == 1);
    CHECK(std::string(a[0]) == "x");
}
