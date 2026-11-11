#pragma once
// The preprocessor runs before the compiler, on text. Each macro here is
// wrong or missing. Fix them so the tests pass.

#include "show.hpp"

// x times itself, for any expression x: SQUARE(1 + 2) is 9.
#define SQUARE(x) ((x) * (x))

// How many elements the array arr has. arr is a real array, not a pointer.
#define COUNT_OF(arr) (sizeof(arr) / sizeof((arr)[0]))

// Turns its argument into a string literal, after expanding any macros in
// it: STR(__LINE__) is "12" on line 12, not "__LINE__".
#define STR_NO_EXPAND(x) #x
#define STR(x) STR_NO_EXPAND(x)

// A string literal "file:line" for the place it's used.
#define HERE __FILE__ ":" STR(__LINE__)

// The expression's text and its value, as a std::string:
// SHOW(6 * 7) is "6 * 7 = 42". Use show() from show.hpp.
#define SHOW(expr) show(#expr, (expr))

// "x86-64" or "arm64", depending on the CPU this is compiled for, or
// "unknown". Decide with #if, at compile time.
inline const char* arch() {
#if defined(__x86_64__)
    return "x86-64";
#elif defined(__aarch64__)
    return "arm64";
#else
    return "unknown";
#endif
}

// "Linux" or "macOS", or "unknown".
inline const char* os() {
#if defined(__linux__)
    return "Linux";
#elif defined(__APPLE__)
    return "macOS";
#else
    return "unknown";
#endif
}
