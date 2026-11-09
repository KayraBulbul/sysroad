#include "heap.hpp"

#include <algorithm>
#include <vector>

#include "alloc_count.hpp"

int* make_filled(std::size_t n, int value) {
    // TODO
    (void)n;
    (void)value;
    return nullptr;
}

int* grow(int* old, std::size_t old_n, std::size_t new_n) {
    // TODO: allocate, copy, zero the rest, free the old array.
    (void)old_n;
    (void)new_n;
    return old;
}

long vector_allocations(std::size_t n, bool reserve_first) {
    // TODO: count with an AllocScope.
    (void)n;
    (void)reserve_first;
    return 0;
}
