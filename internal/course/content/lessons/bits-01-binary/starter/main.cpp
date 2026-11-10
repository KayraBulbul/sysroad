// bits A B: two numbers in binary, and the bitwise operators on them.
// You won't need to change this file.
#include <cstdint>
#include <format>
#include <iostream>

#include "bits.hpp"

static void show(const char* label, std::uint32_t x) {
    std::cout << std::format("{:<5} = {}  0x{:08x}  {}\n", label, to_binary(x), x, x);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: bits A B\n";
        return 2;
    }
    auto a = parse_number(argv[1]);
    auto b = parse_number(argv[2]);
    if (!a || !b) {
        std::cerr << "bits: '" << argv[a ? 2 : 1] << "' isn't a 32-bit number\n";
        return 1;
    }
    show("a", *a);
    show("b", *b);
    show("a & b", *a & *b);
    show("a | b", *a | *b);
    show("a ^ b", *a ^ *b);
    show("~a", ~*a);
    std::cout << std::format("a has {} one bit{} and is {}a power of two\n", count_ones(*a),
                             count_ones(*a) == 1 ? "" : "s", is_power_of_two(*a) ? "" : "not ");
}
