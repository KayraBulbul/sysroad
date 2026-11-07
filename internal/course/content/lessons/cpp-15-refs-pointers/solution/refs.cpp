#include "refs.hpp"

#include <string>
#include <vector>

void swap_values(int& a, int& b) {
    int tmp = a;
    a = b;
    b = tmp;
}

int* find_value(std::vector<int>& v, int target) {
    for (int& x : v) {
        if (x == target) {
            return &x;
        }
    }
    return nullptr;
}

void trim(std::string& s) {
    const char* space = " \t\n\r\f\v";
    std::size_t first = s.find_first_not_of(space);
    if (first == std::string::npos) {
        s.clear();
        return;
    }
    std::size_t last = s.find_last_not_of(space);
    s = s.substr(first, last - first + 1);
}

const std::string* longest(const std::vector<std::string>& words) {
    const std::string* best = nullptr;
    for (const std::string& w : words) {
        if (best == nullptr || w.size() > best->size()) {
            best = &w;
        }
    }
    return best;
}
