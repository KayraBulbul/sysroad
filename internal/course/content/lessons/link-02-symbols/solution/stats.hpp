#pragma once

#include <string>
#include <vector>

// How much report() prints: 0 for the mean only, 1 for the range too.
// Declared here, defined once, in stats.cpp.
extern int verbosity;

// The mean of xs, or 0 for none. Defined in a header, so it has to be inline.
inline double mean(const std::vector<double>& xs) {
    double total = 0;
    for (double x : xs) {
        total += x;
    }
    return xs.empty() ? 0 : total / static_cast<double>(xs.size());
}

// The smallest and largest of xs. xs isn't empty.
double smallest(const std::vector<double>& xs);
double largest(const std::vector<double>& xs);
