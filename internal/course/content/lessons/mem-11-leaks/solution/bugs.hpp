#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <string>

// The sum of the whitespace-separated numbers in text, or -1 if any word
// isn't a number.
int sum_numbers(const std::string& text);

// The first line of text, in capitals.
std::string shout(const std::string& text);

// A new int[n] holding src's n values doubled, or nullptr when n is 0. The
// caller delete[]s it.
int* doubled_copy(const int* src, std::size_t n);

// How many different whitespace-separated words text has.
std::size_t count_unique(const std::string& text);
