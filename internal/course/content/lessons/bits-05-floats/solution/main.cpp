// ieee NUMBER | ieee int N: how a float is stored. You won't need to change this file.
#include <bit>
#include <cmath>
#include <cstdlib>
#include <format>
#include <iostream>
#include <string_view>

#include "ieee.hpp"

int main(int argc, char** argv) {
    if (argc == 3 && std::string_view(argv[1]) == "int") {
        auto n = static_cast<std::uint32_t>(std::strtoul(argv[2], nullptr, 0));
        std::uint32_t bits = from_uint(n);
        std::cout << std::format("{} as a float is 0x{:08x}\n", n, bits);
        std::cout << std::format("exactly {:.120g}\n", value_of(bits));
        return 0;
    }
    if (argc != 2) {
        std::cerr << "usage: ieee NUMBER | ieee int N\n";
        return 2;
    }
    auto bits = std::bit_cast<std::uint32_t>(std::strtof(argv[1], nullptr));
    unsigned sign = bits >> 31;
    int exponent = (bits >> 23) & 0xff;
    std::uint32_t fraction = bits & 0x7fffff;
    std::cout << std::format("{} is stored as 0x{:08x}\n", argv[1], bits);
    std::cout << std::format("sign {}  exponent {}  fraction 0x{:06x}\n", sign, exponent, fraction);
    switch (classify(bits)) {
    case FloatClass::Zero:
        std::cout << "zero\n";
        break;
    case FloatClass::Subnormal:
        std::cout << std::format("subnormal: {:.30g} x 2^-126\n", std::ldexp(fraction, -23));
        break;
    case FloatClass::Normal:
        std::cout << std::format("normal: {:.30g} x 2^{}\n", 1 + std::ldexp(fraction, -23),
                                 exponent - 127);
        break;
    case FloatClass::Infinity:
        std::cout << "infinity\n";
        break;
    case FloatClass::NaN:
        std::cout << "not a number\n";
        break;
    }
    std::cout << std::format("exactly {:.120g}\n", value_of(bits));
}
