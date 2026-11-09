#include "heap.hpp"

#include <algorithm>
#include <vector>

#include "alloc_count.hpp"

int* make_filled(std::size_t n, int value) {
    int* p = new int[n];
    std::fill(p, p + n, value);
    return p;
}

int* grow(int* old, std::size_t old_n, std::size_t new_n) {
    int* p = new int[new_n]();
    std::copy(old, old + std::min(old_n, new_n), p);
    delete[] old;
    return p;
}

long vector_allocations(std::size_t n, bool reserve_first) {
    AllocScope scope;
    {
        std::vector<int> v;
        if (reserve_first) {
            v.reserve(n);
        }
        for (std::size_t i = 0; i < n; ++i) {
            v.push_back(int(i));
        }
    }
    return scope.news();
}
