#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <array>

// The total of the ints in [first, last): from first up to, not including, last.
int sum(const int* first, const int* last);

// Reverses [first, last) in place.
void reverse(int* first, int* last);

// A copy of a with every element doubled.
std::array<int, 5> doubled(std::array<int, 5> a);
