#include "parse.hpp"

#include <charconv>
#include <iostream>
#include <limits>
#include <string_view>
#include <vector>

std::optional<int> parse_int(std::string_view s) {
    int n = 0;
    auto [end, ec] = std::from_chars(s.data(), s.data() + s.size(), n);
    if (s.empty() || ec != std::errc() || end != s.data() + s.size()) {
        return std::nullopt;
    }
    return n;
}

std::optional<std::uint64_t> parse_size(std::string_view s) {
    std::uint64_t mult = 1;
    if (!s.empty()) {
        switch (s.back()) {
        case 'k':
        case 'K':
            mult = 1024;
            break;
        case 'm':
        case 'M':
            mult = 1024 * 1024;
            break;
        case 'g':
        case 'G':
            mult = 1024 * 1024 * 1024;
            break;
        }
        if (mult != 1) {
            s.remove_suffix(1);
        }
    }
    std::uint64_t n = 0;
    auto [end, ec] = std::from_chars(s.data(), s.data() + s.size(), n);
    if (s.empty() || ec != std::errc() || end != s.data() + s.size()) {
        return std::nullopt;
    }
    if (n > std::numeric_limits<std::uint64_t>::max() / mult) {
        return std::nullopt;
    }
    return n * mult;
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);
    int status = 0;
    for (std::string_view arg : args) {
        if (auto n = parse_size(arg)) {
            std::cout << *n << '\n';
        } else {
            std::cerr << "size: invalid size '" << arg << "'\n";
            status = 1;
        }
    }
    return status;
}
