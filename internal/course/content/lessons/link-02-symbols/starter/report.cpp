#include "report.hpp"

#include <cmath>
#include <format>

#include "stats.hpp"

// A helper for this file only: a < b, once both are rounded to two decimals.
bool less_than(double a, double b) {
    return std::round(a * 100) < std::round(b * 100);
}

std::string report(std::vector<double>& xs) {
    if (xs.empty()) {
        return "";
    }
    std::string out = std::format("mean {:.2f}", mean(xs));
    if (verbosity >= 1) {
        double lo = smallest(xs), hi = largest(xs);
        if (less_than(lo, hi)) {
            out += std::format(", range {:.2f} to {:.2f}", lo, hi);
        } else {
            out += ", all equal";  // as far as two decimals can tell
        }
    }
    return out;
}
