// You won't need to change this file.
#include "pixel.hpp"

std::uint8_t brighten(std::uint8_t p, int amount) {
    int v = p + amount;
    if (v < 0) {
        v = 0;
    }
    if (v > 255) {
        v = 255;
    }
    return static_cast<std::uint8_t>(v);
}
