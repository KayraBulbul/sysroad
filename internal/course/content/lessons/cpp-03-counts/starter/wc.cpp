#include "wc.hpp"

#include <array>
#include <cctype>
#include <format>
#include <iostream>
#include <string_view>

Counts count_stream(std::istream& in) {
    Counts c;
    std::array<char, 4096> buf;
    // TODO: read 4096 bytes at a time and count lines, words and bytes
    return c;
}

int main() {
    // TODO: count std::cin and print the three columns
    return 0;
}
