#pragma once
// The preprocessor runs before the compiler, on text. Each macro here is
// wrong or missing. Fix them so the tests pass.

#include "show.hpp"

// x times itself, for any expression x: SQUARE(1 + 2) is 9.
#define SQUARE(x) x * x

// How many elements the array arr has. arr is a real array, not a pointer.
#define COUNT_OF(arr) 0

// Turns its argument into a string literal, after expanding any macros in
// it: STR(__LINE__) is "12" on line 12, not "__LINE__".
#define STR(x) #x

// A string literal "file:line" for the place it's used.
#define HERE ""

// The expression's text and its value, as a std::string:
// SHOW(6 * 7) is "6 * 7 = 42". Use show() from show.hpp.
#define SHOW(expr) std::string()

// "x86-64" or "arm64", depending on the CPU this is compiled for, or
// "unknown". Decide with #if, at compile time.
inline const char* arch() {
    return "unknown";
}

// "Linux" or "macOS", or "unknown".
inline const char* os() {
    return "unknown";
}
