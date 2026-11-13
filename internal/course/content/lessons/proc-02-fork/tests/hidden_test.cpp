#include "doctest.h"

#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "spawn.hpp"

static std::string slurp(const char* path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void append(const char* path, const std::string& line) {
    std::ofstream(path, std::ios::app) << line << '\n';
}

TEST_CASE("the child's exception stays in the child") {
    std::remove("after.txt");
    pid_t pid = spawn([]() -> int { throw std::runtime_error("boom"); });
    // If the exception escaped spawn in the child, the child would run this
    // line too.
    append("after.txt", std::to_string(::getpid()));
    CHECK(wait_exit_code(pid) == 1);
    CHECK(slurp("after.txt") == std::to_string(::getpid()) + "\n");
}

TEST_CASE("buffered output isn't written twice") {
    std::FILE* f = std::fopen("once.txt", "w");
    REQUIRE(f != nullptr);
    std::fputs("once\n", f);  // still in f's buffer
    pid_t pid = spawn([] { return 0; });
    CHECK(wait_exit_code(pid) == 0);
    std::fclose(f);
    CHECK(slurp("once.txt") == "once\n");
}

TEST_CASE("the child's own buffered output isn't lost") {
    std::FILE* f = std::fopen("child.txt", "w");
    REQUIRE(f != nullptr);
    pid_t pid = spawn([f] {
        std::fputs("from the child\n", f);
        return 0;
    });
    CHECK(wait_exit_code(pid) == 0);
    std::fclose(f);
    CHECK(slurp("child.txt") == "from the child\n");
}

TEST_CASE("only the children run the body, once each") {
    std::remove("who.txt");
    auto codes = run_all(3, [](int i) {
        append("who.txt", std::to_string(i));
        return 0;
    });
    CHECK(codes == std::vector<int>{0, 0, 0});
    std::string who = slurp("who.txt");
    CHECK(who.size() == 6);
    for (char c : {'0', '1', '2'}) {
        CHECK(who.find(c) != std::string::npos);
    }
}

TEST_CASE("run_all starts every child before waiting") {
    auto start = std::chrono::steady_clock::now();
    auto codes = run_all(5, [](int i) {
        ::usleep(300'000);
        return i;
    });
    auto took = std::chrono::steady_clock::now() - start;
    CHECK(codes == std::vector<int>{0, 1, 2, 3, 4});
    CHECK(took < std::chrono::milliseconds(1200));
}

TEST_CASE("an exit code is one byte") {
    CHECK(wait_exit_code(spawn([] { return 300; })) == 300 % 256);
}

TEST_CASE("no children are left behind") {
    run_all(3, [](int) { return 0; });
    int status = 0;
    CHECK(::waitpid(-1, &status, WNOHANG) == -1);  // ECHILD: none at all
}
