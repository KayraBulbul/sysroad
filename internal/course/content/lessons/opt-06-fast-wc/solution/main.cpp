// wc, plus a race against your system's wc. You won't need to change this
// file.
//
//   wc [-lwc] [FILE]...   counts, like lesson 1.1.7's wc
//   wc --race MB          writes MB megabytes of text to a temporary file,
//                         then times your wc and the system's on it
#include <fcntl.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "wc.hpp"

static double time_ms(const std::function<void()>& f) {
    double best = 1e18;
    for (int run = 0; run < 3; ++run) {
        auto start = std::chrono::steady_clock::now();
        f();
        std::chrono::duration<double, std::milli> took = std::chrono::steady_clock::now() - start;
        best = std::min(best, took.count());
    }
    return best;
}

static int race(std::size_t mb) {
    char path[] = "/tmp/sysroad-wc-XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) {
        std::perror("mkstemp");
        return 1;
    }
    // Lines of 0 to 14 short words, from a fixed sequence of numbers.
    static const char* words[] = {"the", "quick", "brown", "fox", "jumps", "over", "a", "lazy",
                                  "dog", "cache", "line", "memory", "systems", "I", "branch"};
    std::string text;
    std::uint32_t s = 1;
    auto next = [&] { return s = s * 1664525 + 1013904223, s >> 16; };
    while (text.size() < (mb << 20)) {
        for (std::uint32_t w = next() % 15; w > 0; --w) {
            text += words[next() % 15];
            text += w > 1 ? " " : "";
        }
        text += '\n';
    }
    if (write(fd, text.data(), text.size()) != static_cast<ssize_t>(text.size())) {
        std::perror("write");
        return 1;
    }
    close(fd);

    std::printf("%zu MB of text\n", text.size() >> 20);
    for (const char* flags : {"", "-l "}) {
        std::string cmd = std::string("wc ") + flags + path + " > /dev/null";
        double theirs = time_ms([&] {
            if (std::system(cmd.c_str()) != 0) {
                std::fprintf(stderr, "wc failed\n");
            }
        });
        double yours = time_ms([&] {
            int f = open(path, O_RDONLY);
            Counts c;
            count_fd(f, c);
            close(f);
        });
        std::printf("wc %-3s  system %7.1f ms   yours %7.1f ms  (%.1f GB/s)\n", flags, theirs, yours,
                    static_cast<double>(text.size()) / (yours * 1e6));
    }
    std::printf("(the system's time includes starting a process, about 1 ms)\n");
    unlink(path);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 3 && std::strcmp(argv[1], "--race") == 0) {
        return race(std::strtoull(argv[2], nullptr, 10));
    }
    std::vector<std::string_view> args(argv + 1, argv + argc);
    return run_wc(args, 0, std::cout, std::cerr);
}
