// Shows decay, then tries your functions. You won't need to change this file.
#include <cstdio>

#include "arrays.hpp"

// clang would warn that sizeof here measures a pointer. That's the point.
#pragma clang diagnostic ignored "-Wsizeof-array-argument"

static void show(const int values[5]) {
    std::printf("in a function: sizeof values = %zu (a pointer)\n", sizeof values);
}

int main() {
    int scores[5] = {1, 2, 3, 4, 5};
    std::printf("in main:       sizeof scores = %zu (5 ints)\n", sizeof scores);
    show(scores);

    std::printf("sum of all 5:        %d\n", sum(scores, scores + 5));
    std::printf("sum of the middle 3: %d\n", sum(scores + 1, scores + 4));

    reverse(scores, scores + 5);
    std::printf("reversed:           ");
    for (int s : scores) {
        std::printf(" %d", s);
    }

    std::array<int, 5> a = {1, 2, 3, 4, 5};
    std::array<int, 5> d = doubled(a);
    std::printf("\ndoubled:            ");
    for (int x : d) {
        std::printf(" %d", x);
    }
    std::printf("\nthe original:       ");
    for (int x : a) {
        std::printf(" %d", x);
    }
    std::printf("\n");
}
