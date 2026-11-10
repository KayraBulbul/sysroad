// The two functions the listings call. You won't need to change this file.
#include "mystery.hpp"

long square(long x) {
    return x * x;
}

// Writes four values: seed, 2 * seed, 3 * seed and 4 * seed.
void fill(long* out, long seed) {
    for (long i = 0; i < 4; ++i) {
        out[i] = seed * (i + 1);
    }
}
