#include "fields.hpp"

std::uint32_t get_field(std::uint32_t x, int lo, int width) {
    // TODO: shift the field down to bit 0, then mask off everything above it.
    (void)x;
    (void)lo;
    (void)width;
    return 0;
}

std::uint32_t set_field(std::uint32_t x, int lo, int width, std::uint32_t value) {
    // TODO: clear the field, then OR in the new value.
    (void)lo;
    (void)width;
    (void)value;
    return x;
}

Rgb unpack565(std::uint16_t pixel) {
    // TODO
    (void)pixel;
    return {};
}

std::uint16_t pack565(Rgb colour) {
    // TODO
    (void)colour;
    return 0;
}

std::string mode_string(unsigned mode) {
    // TODO
    (void)mode;
    return "";
}
