#include "twos.hpp"

#include <algorithm>

std::int32_t sign_extend(std::uint32_t x, int bits) {
    if (bits < 32) {
        x &= (1u << bits) - 1;
    }
    std::uint32_t sign = 1u << (bits - 1);
    return static_cast<std::int32_t>((x ^ sign) - sign);
}

bool add_overflows(std::int32_t a, std::int32_t b) {
    std::int64_t sum = std::int64_t{a} + b;
    return sum > INT32_MAX || sum < INT32_MIN;
}

std::int32_t wrapping_add(std::int32_t a, std::int32_t b) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b));
}

std::int32_t saturating_add(std::int32_t a, std::int32_t b) {
    std::int64_t sum = std::int64_t{a} + b;
    return static_cast<std::int32_t>(
        std::clamp<std::int64_t>(sum, std::int64_t{INT32_MIN}, std::int64_t{INT32_MAX}));
}

std::optional<std::int32_t> checked_mul(std::int32_t a, std::int32_t b) {
    std::int64_t product = std::int64_t{a} * b;
    if (product > INT32_MAX || product < INT32_MIN) {
        return std::nullopt;
    }
    return static_cast<std::int32_t>(product);
}
