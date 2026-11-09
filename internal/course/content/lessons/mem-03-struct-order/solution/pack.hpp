#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <vector>

#include "layout.hpp"

// The same fields, largest alignment first. Fields with equal alignment
// keep their order.
std::vector<Field> packed(std::vector<Field> fields);
