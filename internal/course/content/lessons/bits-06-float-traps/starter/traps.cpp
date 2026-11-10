#include "traps.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <string>

bool close_enough(double a, double b) {
    return a == b;
}

double elapsed_seconds(int ticks) {
    float seconds = 0;
    for (int i = 0; i < ticks; ++i) {
        seconds += 0.1f;
    }
    return seconds;
}

double largest(const std::vector<double>& v) {
    if (v.empty()) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    double best = v[0];
    for (double x : v) {
        if (x > best) {
            best = x;
        }
    }
    return best;
}

std::int64_t to_cents(double dollars) {
    return static_cast<std::int64_t>(dollars * 100);
}

std::optional<std::uint64_t> parse_id(std::string_view digits) {
    auto is_digit = [](unsigned char c) { return std::isdigit(c) != 0; };
    if (digits.empty() || !std::all_of(digits.begin(), digits.end(), is_digit)) {
        return std::nullopt;
    }
    double value = std::stod(std::string(digits));
    if (value >= 18446744073709551616.0) {  // 2^64
        return std::nullopt;
    }
    return static_cast<std::uint64_t>(value);
}
