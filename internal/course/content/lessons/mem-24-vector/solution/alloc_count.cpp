// Replaces the global operator new and delete with versions that count, then
// call malloc and free. Leak checkers and memory profilers hook in the same
// way. You won't need to change this file.
#include "alloc_count.hpp"

#include <cstdlib>
#include <new>

namespace {
AllocStats stats;
}

AllocStats alloc_stats() {
    return stats;
}

void* operator new(std::size_t n) {
    ++stats.news;
    stats.bytes += n;
    if (void* p = std::malloc(n == 0 ? 1 : n)) {
        return p;
    }
    throw std::bad_alloc();
}

void* operator new[](std::size_t n) {
    return ::operator new(n);
}

void operator delete(void* p) noexcept {
    if (p != nullptr) {
        ++stats.deletes;
        std::free(p);
    }
}

void operator delete[](void* p) noexcept {
    ::operator delete(p);
}

void operator delete(void* p, std::size_t) noexcept {
    ::operator delete(p);
}

void operator delete[](void* p, std::size_t) noexcept {
    ::operator delete(p);
}
