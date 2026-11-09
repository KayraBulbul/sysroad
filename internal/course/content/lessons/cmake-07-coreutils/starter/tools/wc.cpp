// wc [FILE...]: lines, words and bytes of each file (or standard input),
// and a total for more than one. You won't need to change this file.
#include <format>
#include <iostream>

#include "coreutils/io.hpp"

struct Counts {
    long lines = 0, words = 0, bytes = 0;
};

static void print(const Counts& c, std::string_view name) {
    std::cout << std::format("{:>7} {:>7} {:>7}", c.lines, c.words, c.bytes);
    if (name != "-") {
        std::cout << ' ' << name;
    }
    std::cout << '\n';
}

int main(int argc, char** argv) {
    auto files = coreutils::args(argc, argv);
    Counts total;
    int status =
        coreutils::for_each_input("wc", files, [&](std::istream& in, std::string_view name) {
            Counts c;
            bool in_word = false;
            char ch;
            while (in.get(ch)) {
                ++c.bytes;
                if (ch == '\n') {
                    ++c.lines;
                }
                bool space =
                    ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == '\v' || ch == '\f';
                if (space) {
                    in_word = false;
                } else if (!in_word) {
                    in_word = true;
                    ++c.words;
                }
            }
            print(c, name);
            total.lines += c.lines;
            total.words += c.words;
            total.bytes += c.bytes;
        });
    if (files.size() > 1) {
        print(total, "total");
    }
    return status;
}
