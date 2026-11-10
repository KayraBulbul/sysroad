#include "bits.hpp"

std::string to_binary(std::uint32_t x) {
    // TODO: 32 digits, highest bit first, a space between groups of four.
    (void)x;
    return "";
}

std::optional<std::uint32_t> parse_number(std::string_view s) {
    // TODO: pick the base from the prefix, then value = value * base + digit.
    (void)s;
    return std::nullopt;
}

int count_ones(std::uint32_t x) {
    // TODO
    (void)x;
    return 0;
}

bool is_power_of_two(std::uint32_t x) {
    // TODO
    (void)x;
    return false;
}
