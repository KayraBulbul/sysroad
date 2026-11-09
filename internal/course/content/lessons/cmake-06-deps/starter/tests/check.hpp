#pragma once
// A test helper small enough to read: CHECK(cond) reports a failing line
// and counts it, and check_result() is the exit status for main to return.
// You won't need to change this file.
#include <cstdio>

inline int& check_failures() {
    static int failures = 0;
    return failures;
}

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond);          \
            ++check_failures();                                                                    \
        }                                                                                          \
    } while (0)

inline int check_result() {
    return check_failures() == 0 ? 0 : 1;
}
