// twos BITS VALUE | twos add A B. You won't need to change this file.
#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

#include "twos.hpp"

static std::string binary(std::uint32_t x, int bits) {
    std::string out;
    for (int i = bits - 1; i >= 0; --i) {
        out += (x >> i) & 1 ? '1' : '0';
        if (i % 4 == 0 && i != 0) {
            out += ' ';
        }
    }
    return out;
}

int main(int argc, char** argv) {
    if (argc == 4 && std::string_view(argv[1]) == "add") {
        auto a = static_cast<std::int32_t>(std::strtol(argv[2], nullptr, 0));
        auto b = static_cast<std::int32_t>(std::strtol(argv[3], nullptr, 0));
        if (!add_overflows(a, b)) {
            std::cout << std::format("{} + {} = {}\n", a, b, wrapping_add(a, b));
            return 0;
        }
        std::cout << std::format("{} + {} overflows 32 bits\n", a, b);
        std::cout << std::format("it wraps to {} and saturates at {}\n", wrapping_add(a, b),
                                 saturating_add(a, b));
        return 0;
    }
    if (argc != 3) {
        std::cerr << "usage: twos BITS VALUE | twos add A B\n";
        return 2;
    }
    int bits = std::atoi(argv[1]);
    if (bits < 1 || bits > 32) {
        std::cerr << "twos: BITS must be 1 to 32\n";
        return 2;
    }
    auto raw = static_cast<std::uint32_t>(std::strtoll(argv[2], nullptr, 0));
    std::uint32_t low = bits == 32 ? raw : raw & ((1u << bits) - 1);
    std::cout << std::format("{} in {} bits is {}\n", argv[2], bits, binary(low, bits));
    std::cout << std::format("unsigned {}, signed {}\n", low, sign_extend(raw, bits));
}
