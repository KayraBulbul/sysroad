#include "mystery.hpp"

long foxtrot(long a, long b, long c, long d, long e, long f, long g, long h) {
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return (a + g) * h;
}

long golf(long x) {
    return square(x) + 1;
}

long hotel(long a, long b) {
    return square(a) + square(b);
}

long india(long n) {
    if (n <= 0) {
        return 0;
    }
    return square(india(n - 1)) + n;
}

long juliet(long seed) {
    long values[4];
    fill(values, seed);
    return values[0] + values[3];
}
