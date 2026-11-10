---
title: A Bitset class
minutes: 45
files: [bitset.cpp]
program: primes
run:
  args: ["50"]
deeper:
  - title: std::bitset (cppreference)
    url: https://en.cppreference.com/w/cpp/utility/bitset
  - title: "The <bit> header (cppreference)"
    url: https://en.cppreference.com/w/cpp/header/bit
  - title: Sieve of Eratosthenes (Wikipedia)
    url: https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
---
A `std::vector<bool>` of a million flags could take a million bytes.
Packed one flag per bit, it takes 125 KB, fits in the CPU's cache, and
lets you combine two sets 64 flags at a time with a single `&` or `|`.
That's a **bitset**, and it's behind database indexes, Bloom filters,
memory allocators' free maps and chess engines.

This project puts the chapter together. Your `Bitset` has a size
chosen at run time and stores its bits in 64-bit words:

```cpp
std::size_t size_;                  // how many bits
std::vector<std::uint64_t> words_;  // bit i lives in words_[i / 64], at position i % 64
```

Everything you need for one bit is in lesson 4.1.2. For whole words,
`<bit>` has the two functions that make a bitset fast, and both
compile to a single instruction:

```cpp
std::popcount(word)       // how many 1 bits
std::countr_zero(word)    // how many 0s below the lowest 1: the index of the lowest set bit
```

There is one rule to keep, and most of the bugs in a bitset come from
forgetting it. A 70-bit set uses two words, so 58 bits of the second
word are **unused**. Keep them 0 at all times. Then `count`, `any`,
`==` and the searches can work on whole words without thinking about
the size. Two operations can break the rule: `set_all` and `~`.

`bitset.hpp` has the class and says what every function does.
`main.cpp` is done. It's the sieve of Eratosthenes, which starts with
every number marked and crosses off the multiples of each prime:

```
$ ./primes 50
2 3 5 7 11 13 17 19 23 29 31 37 41 43 47
15 primes below 50
```

## Assignment

- [ ] The constructor, `test`, `set` and `flip`, throwing `std::out_of_range` for a bad index
- [ ] `set_all`, `count` and `any`, with the unused bits kept at 0
- [ ] `find_first` and `find_next` skip empty words instead of testing bit by bit
- [ ] `&=`, `|=`, `^=` and `~` work a word at a time
- [ ] `to_string` writes bit 0 first
