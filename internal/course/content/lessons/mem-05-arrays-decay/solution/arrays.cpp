#include "arrays.hpp"

#include <utility>

int sum(const int* first, const int* last) {
    int total = 0;
    for (const int* p = first; p != last; ++p) {
        total += *p;
    }
    return total;
}

void reverse(int* first, int* last) {
    while (first != last && first != --last) {
        std::swap(*first, *last);
        ++first;
    }
}

std::array<int, 5> doubled(std::array<int, 5> a) {
    for (int& x : a) {
        x *= 2;
    }
    return a;
}
