// fields rgb PIXEL | fields pack R G B | fields mode OCTAL
// You won't need to change this file.
#include <cstdlib>
#include <format>
#include <iostream>
#include <string_view>

#include "fields.hpp"

static unsigned long number(const char* s, int base) {
    return std::strtoul(s, nullptr, base);
}

int main(int argc, char** argv) {
    std::string_view cmd = argc > 1 ? argv[1] : "";
    if (cmd == "rgb" && argc == 3) {
        Rgb c = unpack565(static_cast<std::uint16_t>(number(argv[2], 0)));
        std::cout << std::format("r {}  g {}  b {}\n", c.r, c.g, c.b);
    } else if (cmd == "pack" && argc == 5) {
        Rgb c{static_cast<std::uint8_t>(number(argv[2], 0)),
              static_cast<std::uint8_t>(number(argv[3], 0)),
              static_cast<std::uint8_t>(number(argv[4], 0))};
        std::cout << std::format("0x{:04x}\n", pack565(c));
    } else if (cmd == "mode" && argc == 3) {
        std::cout << mode_string(static_cast<unsigned>(number(argv[2], 8))) << '\n';
    } else {
        std::cerr << "usage: fields rgb PIXEL | fields pack R G B | fields mode OCTAL\n";
        return 2;
    }
}
