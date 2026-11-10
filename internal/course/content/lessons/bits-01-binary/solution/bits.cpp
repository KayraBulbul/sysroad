#include "bits.hpp"

std::string to_binary(std::uint32_t x) {
    std::string out;
    for (int i = 31; i >= 0; --i) {
        if (i % 4 == 3 && i != 31) {
            out += ' ';
        }
        out += (x >> i) & 1 ? '1' : '0';
    }
    return out;
}

static int digit_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return 99;
}

std::optional<std::uint32_t> parse_number(std::string_view s) {
    unsigned base = 10;
    if (s.starts_with("0x") || s.starts_with("0X")) {
        base = 16;
        s.remove_prefix(2);
    } else if (s.starts_with("0b") || s.starts_with("0B")) {
        base = 2;
        s.remove_prefix(2);
    }
    if (s.empty()) {
        return std::nullopt;
    }
    std::uint64_t value = 0;
    for (char c : s) {
        unsigned digit = static_cast<unsigned>(digit_value(c));
        if (digit >= base) {
            return std::nullopt;
        }
        value = value * base + digit;
        if (value > 0xffffffff) {
            return std::nullopt;
        }
    }
    return static_cast<std::uint32_t>(value);
}

int count_ones(std::uint32_t x) {
    int n = 0;
    while (x != 0) {
        x &= x - 1;
        ++n;
    }
    return n;
}

bool is_power_of_two(std::uint32_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}
