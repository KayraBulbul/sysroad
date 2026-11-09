// Tries the list. You won't need to change this file.
#include <cstdio>

#include "list.hpp"

static void print(const char* label, const IntList& l) {
    std::printf("%-12s", label);
    for (int v : l.to_vector()) {
        std::printf(" %d", v);
    }
    std::printf("  (size %zu)\n", l.size());
}

int main() {
    IntList l;
    for (int i = 1; i <= 5; ++i) {
        l.push_back(i);
    }
    l.push_front(0);
    print("built:", l);
    l.reverse();
    print("reversed:", l);
    if (!l.empty()) {
        int first = l.pop_front();
        std::printf("pop_front:   %d\n", first);
    }
    print("now:", l);
    std::printf("contains 3:  %s\n", l.contains(3) ? "yes" : "no");
    std::fflush(stdout);

    {
        IntList big;
        for (int i = 0; i < 1'000'000; ++i) {
            big.push_back(i);
        }
        std::printf("\na big list:  size %zu\n", big.size());
        std::fflush(stdout);
    }
    std::printf("...and it was freed without running out of stack\n");
}
