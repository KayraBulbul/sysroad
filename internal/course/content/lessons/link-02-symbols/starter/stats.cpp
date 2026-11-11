#include "stats.hpp"

#include <algorithm>

// A helper for this file only.
bool less_than(double a, double b) {
    return a < b;
}

double smallest(const std::vector<double>& xs) {
    return *std::min_element(xs.begin(), xs.end(), less_than);
}

double largest(const std::vector<double>& xs) {
    return *std::max_element(xs.begin(), xs.end(), less_than);
}
