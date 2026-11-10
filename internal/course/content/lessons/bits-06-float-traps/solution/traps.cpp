#include "traps.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>

bool close_enough(double a, double b) {
    if (a == b) {
        return true;
    }
    if (std::isinf(a) || std::isinf(b)) {
        return false;
    }
    double scale = std::max({std::abs(a), std::abs(b), 1.0});
    return std::abs(a - b) <= 1e-9 * scale;
}

double elapsed_seconds(int ticks) {
    return ticks / 10.0;
}

double largest(const std::vector<double>& v) {
    double best = std::numeric_limits<double>::quiet_NaN();
    for (double x : v) {
        if (!std::isnan(x) && (std::isnan(best) || x > best)) {
            best = x;
        }
    }
    return best;
}

std::int64_t to_cents(double dollars) {
    return std::llround(dollars * 100);
}

std::optional<std::uint64_t> parse_id(std::string_view digits) {
    std::uint64_t value = 0;
    const char* end = digits.data() + digits.size();
    auto [stop, error] = std::from_chars(digits.data(), end, value);
    if (digits.empty() || error != std::errc{} || stop != end) {
        return std::nullopt;
    }
    return value;
}
