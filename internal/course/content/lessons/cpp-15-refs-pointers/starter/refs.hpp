#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <string>
#include <vector>

// Swaps a and b.
void swap_values(int& a, int& b);

// The first element equal to target, or nullptr.
int* find_value(std::vector<int>& v, int target);

// Removes leading and trailing whitespace, in place.
void trim(std::string& s);

// The longest word (the first one on a tie), or nullptr when there are none.
const std::string* longest(const std::vector<std::string>& words);
