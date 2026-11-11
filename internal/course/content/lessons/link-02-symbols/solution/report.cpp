#include "report.hpp"

#include <format>

#include "stats.hpp"

namespace {

// A helper for this file only: a < b, as strings of two decimals.
bool less_than(double a, double b) {
    return std::format("{:.2f}", a) < std::format("{:.2f}", b);
}

}  // namespace

std::string report(const std::vector<double>& xs) {
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
