#include "mystery.hpp"

unsigned quebec(unsigned x) {
    return x / 10;
}

long romeo(long x) {
    return x * 15;
}

int sierra(int x) {
    return x / 4;
}

bool tango(char c) {
    return c >= '0' && c <= '9';
}

// The number of days before a month starts, in a year that isn't a leap year.
int uniform(int month) {
    switch (month) {
    case 1:
        return 0;
    case 2:
        return 31;
    case 3:
        return 59;
    case 4:
        return 90;
    case 5:
        return 120;
    case 6:
        return 151;
    case 7:
        return 181;
    case 8:
        return 212;
    case 9:
        return 243;
    case 10:
        return 273;
    case 11:
        return 304;
    case 12:
        return 334;
    default:
        return -1;
    }
}
