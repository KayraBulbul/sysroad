#pragma once

#include <string>
#include <vector>

// "mean 2.50" at verbosity 0, "mean 2.50, range 1.00 to 4.00" at 1, or
// "mean 2.00, all equal" at 1 when the range shows as one number at two
// decimals. Nothing at all for no values.
std::string report(const std::vector<double>& xs);
