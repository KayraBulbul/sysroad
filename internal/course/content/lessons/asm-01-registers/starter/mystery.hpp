#pragma once
// The six functions in the listings. You won't need to change this file.

// extern "C" keeps the names in the listings plain: `third` instead of
// `_Z5thirdl`. Lesson 4.2.2 explains.
extern "C" {
long first(long a, long b);
long second(long a, long b);
long third(long a);
long fourth(long a, long b, long c);
int fifth(int a, int b);
long sixth(long a, long b);
}
