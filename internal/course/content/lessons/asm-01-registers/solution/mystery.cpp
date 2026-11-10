#include "mystery.hpp"

long first(long a, long b) {
    return a + b;
}

long second(long a, long b) {
    return a - 2 * b;
}

long third(long a) {
    return 5 * a + 7;
}

long fourth(long a, long b, long c) {
    return (a - b) * c;
}

int fifth(int a, int b) {
    return 3 * a - b + 100;
}

long sixth(long a, long b) {
    return (a << 4) + (b >> 2);
}
