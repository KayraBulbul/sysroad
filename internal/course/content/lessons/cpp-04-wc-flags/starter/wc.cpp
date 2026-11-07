#include "wc.hpp"

#include <array>
#include <cctype>
#include <format>
#include <iostream>
#include <string_view>
#include <vector>

Counts count_stream(std::istream& in) {
    Counts c;
    std::array<char, 4096> buf;
    bool in_word = false;
    while (in.read(buf.data(), buf.size()) || in.gcount() > 0) {
        std::string_view chunk(buf.data(), static_cast<std::size_t>(in.gcount()));
        c.bytes += chunk.size();
        for (char ch : chunk) {
            if (ch == '\n') {
                ++c.lines;
            }
            bool space = std::isspace(static_cast<unsigned char>(ch));
            if (!space && !in_word) {
                ++c.words;
            }
            in_word = !space;
        }
    }
    return c;
}

Options parse_options(const std::vector<std::string_view>& args) {
    Options o;
    // TODO: read the flags
    return o;
}

std::string format_counts(const Counts& c, const Options& o) {
    // TODO: the chosen columns
    return "";
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    // TODO: parse the flags, report a bad one, count std::cin and print
    return 0;
}
