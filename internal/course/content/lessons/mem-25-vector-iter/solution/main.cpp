// Uses Vector with the standard algorithms. You won't need to change this file.
#include <algorithm>
#include <cstdio>

#include "vector.hpp"

static void print(const char* label, const Vector<int>& v) {
    std::printf("%-20s", label);
    for (int x : v) {
        std::printf(" %d", x);
    }
    std::printf("\n");
}

int main() {
    Vector<int> v = {5, 3, 9, 1, 7};
    print("start:", v);
    std::sort(v.begin(), v.end());
    print("std::sort:", v);
    v.insert(v.begin(), 0);
    v.insert(v.begin() + 3, 4);
    v.insert(v.end(), 10);
    print("insert 0, 4, 10:", v);

    std::size_t n = erase_if(v, [](int x) { return x % 2 == 1; });
    print("erase_if odd:", v);
    std::printf("%-20s %zu\n", "  removed:", n);

    for (auto it = v.begin(); it != v.end();) {
        if (*it == 4) {
            it = v.erase(it);  // erase returns the next valid iterator
        } else {
            ++it;
        }
    }
    print("erase the 4:", v);

    const int* before = v.data();
    v.reserve(v.capacity() + 1);
    std::printf("\nafter growing, the buffer %s\n",
                v.data() == before ? "stayed put" : "moved: every old iterator now dangles");
}
