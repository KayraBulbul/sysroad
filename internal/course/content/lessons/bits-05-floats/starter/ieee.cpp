#include "ieee.hpp"

#include <bit>
#include <cmath>
#include <limits>

FloatClass classify(std::uint32_t bits) {
    // TODO: look at the exponent field (0, 255 or neither) and whether the fraction is 0.
    (void)bits;
    return FloatClass::Normal;
}

double value_of(std::uint32_t bits) {
    // TODO: one formula per kind of float. std::ldexp(x, n) is x * 2^n.
    (void)bits;
    return 0.0;
}

std::uint32_t from_uint(std::uint32_t n) {
    // TODO: find the highest set bit, keep 24 bits, round the rest.
    (void)n;
    return 0;
}
