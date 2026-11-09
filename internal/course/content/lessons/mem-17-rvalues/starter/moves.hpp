#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <vector>

#include "tracked.hpp"

// Adds item to the end of out.
void append(std::vector<Tracked>& out, Tracked item);

// Removes the last element of v (which isn't empty) and returns it.
Tracked take_last(std::vector<Tracked>& v);

// a's elements followed by b's.
std::vector<Tracked> concat(std::vector<Tracked> a, std::vector<Tracked> b);

// a gets b's value, b gets c's and c gets a's.
void rotate3(Tracked& a, Tracked& b, Tracked& c);

// The element with the longest value (the first, if there's a tie). v isn't empty.
const Tracked& longest(const std::vector<Tracked>& v);
