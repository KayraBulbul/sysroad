#include "twos.hpp"

std::int32_t sign_extend(std::uint32_t x, int bits) {
    // TODO: keep the low `bits` bits; if the top one of them is set, the value is negative.
    (void)bits;
    return static_cast<std::int32_t>(x);
}

bool add_overflows(std::int32_t a, std::int32_t b) {
    // TODO: decide without ever computing an int sum that overflows.
    (void)a;
    (void)b;
    return false;
}

std::int32_t wrapping_add(std::int32_t a, std::int32_t b) {
    // This is undefined behaviour when the sum doesn't fit. TODO: make it wrap.
    return a + b;
}

std::int32_t saturating_add(std::int32_t a, std::int32_t b) {
    // TODO
    return a + b;
}

std::optional<std::int32_t> checked_mul(std::int32_t a, std::int32_t b) {
    // TODO
    return a * b;
}
