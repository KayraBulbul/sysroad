#include "wc.hpp"

#include <array>
#include <cctype>
#include <format>
#include <iostream>
#include <string_view>

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

int main() {
    Counts c = count_stream(std::cin);
    std::cout << std::format("{:>7} {:>7} {:>7}\n", c.lines, c.words, c.bytes);
    return 0;
}
