// Where do allocations happen? You won't need to change this file.
#include <cstdio>
#include <string>

#include "alloc_count.hpp"
#include "heap.hpp"

static const char* plural(long n) {
    return n == 1 ? "" : "s";
}

static void print(const int* a, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        std::printf("%d ", a[i]);
    }
}

int main() {
    AllocScope s1;
    int* a = make_filled(5, 7);
    std::printf("make_filled(5, 7): ");
    if (a != nullptr) {
        print(a, 5);
    }
    std::printf("  (%ld new%s, %zu bytes)\n", s1.news(), plural(s1.news()), s1.bytes());

    AllocScope s2;
    a = grow(a, 5, 8);
    std::printf("grow to 8:         ");
    if (a != nullptr) {
        print(a, 8);
    }
    std::printf("  (%ld new%s, %ld delete%s)\n", s2.news(), plural(s2.news()), s2.deletes(),
                plural(s2.deletes()));
    delete[] a;

    long plain = vector_allocations(1000, false);
    long reserved = vector_allocations(1000, true);
    std::printf("\npush_back 1000 ints:      %ld allocation%s\n", plain, plural(plain));
    std::printf("the same after reserve:   %ld allocation%s\n", reserved, plural(reserved));

    for (int len : {10, 40}) {
        AllocScope s;
        std::string text(len, 'x');
        std::printf("a %d-char std::string:    %ld allocation%s\n", len, s.news(), plural(s.news()));
    }
}
