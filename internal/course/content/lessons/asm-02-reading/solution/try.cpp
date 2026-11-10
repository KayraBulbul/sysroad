// A scratch file for looking at assembly. In this folder (sysroad path):
//
//   clang++ -std=c++20 -O1 -S try.cpp -o -
//
// Try -O0 and -O2 as well, change the code, and add your own functions.
#include <cstdio>

int square(int x) {
    return x * x;
}

long total(const long* values, long count) {
    long sum = 0;
    for (long i = 0; i < count; ++i) {
        sum += values[i];
    }
    return sum;
}

void hello() {
    std::puts("hello");
}
