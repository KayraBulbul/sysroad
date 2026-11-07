#include "parse.hpp"

#include <charconv>
#include <iostream>
#include <limits>
#include <string_view>
#include <vector>

std::optional<int> parse_int(std::string_view s) {
    return std::nullopt;  // TODO
}

std::optional<std::uint64_t> parse_size(std::string_view s) {
    return std::nullopt;  // TODO
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    // TODO: print each size, or "size: invalid size 'X'" on stderr
    return 0;
}
