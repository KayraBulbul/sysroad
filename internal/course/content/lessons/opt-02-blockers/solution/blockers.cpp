#include "blockers.hpp"

#include <cstring>

void lowercase(char* s) {
    std::size_t n = std::strlen(s);
    for (std::size_t i = 0; i < n; ++i) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = static_cast<char>(s[i] - 'A' + 'a');
        }
    }
}

void invert(Image& img) {
    // Locals can't be changed by the stores below, so the compiler can
    // keep them in registers and vectorise.
    std::uint8_t* p = img.pixels;
    std::size_t n = img.size;
    for (std::size_t i = 0; i < n; ++i) {
        p[i] = static_cast<std::uint8_t>(255 - p[i]);
    }
}

void threshold(std::vector<std::uint8_t>& pixels, std::uint8_t level) {
    // A range-for reads begin() and end() once, before the loop.
    for (std::uint8_t& p : pixels) {
        p = p < level ? 0 : 255;
    }
}
