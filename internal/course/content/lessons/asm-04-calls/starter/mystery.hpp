#pragma once
// The five functions in the listings, and the two they call.
// You won't need to change this file.

extern "C" {
// In helpers.cpp.
long square(long x);
void fill(long* out, long seed);

long foxtrot(long a, long b, long c, long d, long e, long f, long g, long h);
long golf(long x);
long hotel(long a, long b);

// Only called with n <= 7.
long india(long n);

long juliet(long seed);
}
