#pragma once
// Your answers, read from x86-64.s or arm64.s. Replace each "" or 0.

#include <string_view>

// 1. The first function in the listing, as C++: what c++filt prints for
//    its label.
constexpr std::string_view kFirstFunction = "geo::area(int, int)";

// 2. The text that banner() returns a pointer to.
constexpr std::string_view kBanner = "sysroad 4.2";

// 3. The number that answer() returns.
constexpr int kAnswer = 650;

// 4. What the first call to bump() returns.
constexpr long kFirstBump = 43;

// 5. greet() hands a string to one function from the C library. Its name.
constexpr std::string_view kGreetCalls = "puts";
