#include "mystery.hpp"

long alpha(long a, long b) {
    if (a > b) {
        return a;
    }
    return b;
}

int bravo(int x) {
    if (x >= 90) {
        return 4;
    }
    if (x >= 75) {
        return 3;
    }
    if (x >= 50) {
        return 2;
    }
    return 0;
}

long charlie(long a, unsigned long n) {
    long result = 1;
    while (n != 0) {
        result *= a;
        --n;
    }
    return result;
}

long delta(long n) {
    long steps = 0;
    while (n != 1) {
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
        ++steps;
    }
    return steps;
}

unsigned long echo(unsigned long a, unsigned long b) {
    while (b != 0) {
        unsigned long r = a % b;
        a = b;
        b = r;
    }
    return a;
}
