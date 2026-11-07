#include "bugs.hpp"

// Five functions, five bugs. Each one "works" most of the time without
// sanitizers. Run, read the report, fix, repeat.

int sum_first(const std::vector<int>& v, std::size_t n) {
    const int* p = v.data();
    int sum = 0;
    for (std::size_t i = 0; i < n; ++i) {
        sum += p[i];
    }
    return sum;
}

int first_then_grow(std::vector<int>& v) {
    int first = v[0];  // a copy, not a reference into the buffer
    v.push_back(42);
    return first;
}

int average(int a, int b) {
    return static_cast<int>((static_cast<long long>(a) + b) / 2);
}

static std::string greeting(std::string_view name) {
    return "Hello, " + std::string(name) + "! Welcome to the systems course.";
}

std::string welcome(std::string_view name) {
    return greeting(name);
}

int count_digit(std::string_view s, int d) {
    int counts[10] = {};
    for (char c : s) {
        if (c >= '0' && c <= '9') {
            counts[c - '0']++;
        }
    }
    return counts[d];
}
