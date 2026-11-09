// Calls each function once. You won't need to change this file.
#include <cstdio>

#include "bugs.hpp"

int main() {
    std::printf("sum_numbers(\"1 2 3\")   = %d\n", sum_numbers("1 2 3"));
    std::printf("sum_numbers(\"1 two 3\") = %d\n", sum_numbers("1 two 3"));

    std::printf("shout(\"hello\\nworld\")  = %s\n", shout("hello\nworld").c_str());

    int src[] = {1, 2, 3};
    int* d = doubled_copy(src, 3);
    std::printf("doubled_copy(3 ints)   = %d %d %d\n", d[0], d[1], d[2]);
    delete[] d;
    d = doubled_copy(src, 0);
    std::printf("doubled_copy(0 ints)   = %s\n", d == nullptr ? "nullptr" : "a pointer");
    delete[] d;

    std::printf("count_unique(\"a b a c b\") = %zu\n", count_unique("a b a c b"));
}
