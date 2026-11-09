// bytes TYPE VALUE: show how a value is stored. You won't need to change this file.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include "bytes.hpp"

template <typename T> void show(std::string_view type, std::string_view text, T value) {
    std::cout << type << ' ' << text << ": " << sizeof value << " bytes\n"
              << bytes_of(value) << '\n';
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: bytes char|short|int|long|float|double VALUE\n";
        return 2;
    }
    std::string_view type = argv[1];
    std::string text = argv[2];
    try {
        if (type == "char") {
            show(type, text, static_cast<char>(text.empty() ? 0 : text[0]));
        } else if (type == "short") {
            show(type, text, static_cast<short>(std::stoi(text)));
        } else if (type == "int") {
            show(type, text, std::stoi(text));
        } else if (type == "long") {
            show(type, text, std::stoll(text));
        } else if (type == "double") {
            show(type, text, std::stod(text));
        } else if (type == "float") {
            float f = std::stof(text);
            show(type, text, f);
            FloatParts fp = parts_of(f);
            std::cout << "sign " << fp.sign << "  exponent " << fp.exponent;
            if (fp.exponent != 0 && fp.exponent != 255) {
                std::cout << " (2^" << static_cast<int>(fp.exponent) - 127 << ')';
            }
            char hex[16];
            std::snprintf(hex, sizeof hex, "0x%06x", static_cast<unsigned>(fp.mantissa));
            std::cout << "  mantissa " << hex << '\n';
        } else {
            std::cerr << "bytes: unknown type " << type << '\n';
            return 2;
        }
    } catch (const std::exception&) {
        std::cerr << "bytes: can't read " << text << " as " << type << '\n';
        return 1;
    }
}
