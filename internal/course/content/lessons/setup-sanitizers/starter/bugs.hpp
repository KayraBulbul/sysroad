#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <string>
#include <string_view>
#include <vector>

// The sum of the first n elements (n <= v.size()).
int sum_first(const std::vector<int>& v, std::size_t n);

// Appends 42 to v, and returns what v[0] was.
int first_then_grow(std::vector<int>& v);

// The average of a and b, rounded towards zero, for any two ints.
int average(int a, int b);

// "Hello, NAME! Welcome to the systems course." (returned as a std::string)
std::string welcome(std::string_view name);

// How many times digit d (0-9) appears in s. Other characters are ignored.
int count_digit(std::string_view s, int d);
