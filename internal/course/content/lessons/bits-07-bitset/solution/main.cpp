// primes N: the primes below N, by the sieve of Eratosthenes.
// You won't need to change this file.
#include <cstdlib>
#include <iostream>

#include "bitset.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: primes N\n";
        return 2;
    }
    std::size_t n = std::strtoull(argv[1], nullptr, 10);
    Bitset prime(n);
    prime.set_all();
    for (std::size_t i = 0; i < 2 && i < n; ++i) {
        prime.set(i, false);
    }
    for (std::size_t p = 2; p * p < n; p = prime.find_next(p)) {
        for (std::size_t multiple = p * p; multiple < n; multiple += p) {
            prime.set(multiple, false);
        }
    }
    const char* gap = "";
    for (std::size_t p = prime.find_first(); p < n; p = prime.find_next(p)) {
        std::cout << gap << p;
        gap = " ";
    }
    if (prime.any()) {
        std::cout << '\n';
    }
    std::size_t count = prime.count();
    std::cout << count << (count == 1 ? " prime below " : " primes below ") << n << '\n';
}
