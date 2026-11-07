#include "bugs.hpp"

// Five functions, five bugs. Each one "works" most of the time without
// sanitizers. Run, read the report, fix, repeat.

int sum_first(const std::vector<int>& v, std::size_t n) {
    const int* p = v.data();
    int sum = 0;
    for (std::size_t i = 0; i <= n; ++i) {
        sum += p[i];
    }
    return sum;
}

int first_then_grow(std::vector<int>& v) {
    const int& first = v[0];
    v.push_back(42);
    return first;
}

int average(int a, int b) {
    return (a + b) / 2;
}

static std::string_view greeting(std::string_view name) {
    std::string s = "Hello, " + std::string(name) + "! Welcome to the systems course.";
    return s;
}

std::string welcome(std::string_view name) {
    return std::string(greeting(name));
}

int count_digit(std::string_view s, int d) {
    int counts[10] = {};
    for (char c : s) {
        counts[c - '0']++;
    }
    return counts[d];
}
