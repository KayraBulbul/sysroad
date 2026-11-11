// Uses each macro. You won't need to change this file.
#include <cstdio>

#include "macros.hpp"

int main() {
    int primes[] = {2, 3, 5, 7, 11};
    std::printf("built for %s on %s\n", arch(), os());
    std::printf("primes: %d %d %d %d %d\n", primes[0], primes[1], primes[2], primes[3], primes[4]);
    std::printf("SQUARE(1 + 2) * 2 = %d\n", SQUARE(1 + 2) * 2);
    std::printf("COUNT_OF(primes) = %zu\n", static_cast<std::size_t>(COUNT_OF(primes)));
    std::printf("HERE = %s\n", HERE);
    std::printf("%s\n", SHOW(primes[2] * 10 + 1).c_str());
}
