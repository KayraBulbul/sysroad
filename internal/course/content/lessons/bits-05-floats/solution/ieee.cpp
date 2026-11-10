#include "ieee.hpp"

#include <bit>
#include <cmath>
#include <limits>

FloatClass classify(std::uint32_t bits) {
    int exponent = (bits >> 23) & 0xff;
    std::uint32_t fraction = bits & 0x7fffff;
    if (exponent == 0) {
        return fraction == 0 ? FloatClass::Zero : FloatClass::Subnormal;
    }
    if (exponent == 0xff) {
        return fraction == 0 ? FloatClass::Infinity : FloatClass::NaN;
    }
    return FloatClass::Normal;
}

double value_of(std::uint32_t bits) {
    double sign = bits >> 31 ? -1.0 : 1.0;
    int exponent = (bits >> 23) & 0xff;
    std::uint32_t fraction = bits & 0x7fffff;
    switch (classify(bits)) {
    case FloatClass::Zero:
        return sign * 0.0;
    case FloatClass::Subnormal:
        return sign * std::ldexp(static_cast<double>(fraction), -126 - 23);
    case FloatClass::Normal:
        return sign * std::ldexp(static_cast<double>(fraction | 0x800000), exponent - 127 - 23);
    case FloatClass::Infinity:
        return sign * std::numeric_limits<double>::infinity();
    case FloatClass::NaN:
        break;
    }
    return std::numeric_limits<double>::quiet_NaN();
}

std::uint32_t from_uint(std::uint32_t n) {
    if (n == 0) {
        return 0;
    }
    int top = 31 - std::countl_zero(n);  // the highest set bit
    std::uint32_t exponent = 127 + static_cast<std::uint32_t>(top);
    if (top <= 23) {
        return exponent << 23 | ((n << (23 - top)) & 0x7fffff);
    }
    int drop = top - 23;
    std::uint32_t kept = n >> drop;
    std::uint32_t rest = n & ((1u << drop) - 1);
    std::uint32_t half = 1u << (drop - 1);
    if (rest > half || (rest == half && (kept & 1) != 0)) {
        ++kept;
    }
    if (kept == 1u << 24) {  // rounded up to the next power of two
        return (exponent + 1) << 23;
    }
    return exponent << 23 | (kept & 0x7fffff);
}
