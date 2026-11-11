#include "blockers.hpp"

#include <cstring>

// Each of these is correct, and slower than it looks. Keep what they do,
// and change how they do it.

void lowercase(char* s) {
    for (std::size_t i = 0; i < std::strlen(s); ++i) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = static_cast<char>(s[i] - 'A' + 'a');
        }
    }
}

void invert(Image& img) {
    for (std::size_t i = 0; i < img.size; ++i) {
        img.pixels[i] = static_cast<std::uint8_t>(255 - img.pixels[i]);
    }
}

void threshold(std::vector<std::uint8_t>& pixels, std::uint8_t level) {
    for (std::size_t i = 0; i < pixels.size(); ++i) {
        pixels[i] = pixels[i] < level ? 0 : 255;
    }
}
