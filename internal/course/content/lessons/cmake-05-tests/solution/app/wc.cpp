// wc [FILE...]: lines, words and bytes of each file (or standard input),
// and a total for more than one. You won't need to change this file.
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "textutil/textutil.hpp"

static void print(const textutil::Counts& c, std::string_view name) {
    std::cout << std::format("{:>7} {:>7} {:>7}", c.lines, c.words, c.bytes);
    if (!name.empty()) {
        std::cout << ' ' << name;
    }
    std::cout << '\n';
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::string text{std::istreambuf_iterator<char>(std::cin), {}};
        print(textutil::count(text), "");
        return 0;
    }
    int status = 0;
    std::vector<textutil::Counts> all;
    for (int i = 1; i < argc; ++i) {
        std::ifstream in(argv[i], std::ios::binary);
        if (!in) {
            std::cerr << "wc: " << argv[i] << ": can't open\n";
            status = 1;
            continue;
        }
        std::string text{std::istreambuf_iterator<char>(in), {}};
        all.push_back(textutil::count(text));
        print(all.back(), argv[i]);
    }
    if (argc > 2) {
        print(textutil::total(all), "total");
    }
    return status;
}
