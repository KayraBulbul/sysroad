---
title: "Binary, hex and bitwise operators"
minutes: 20
files: [bits.cpp]
run:
  args: ["202", "0x0f"]
deeper:
  - title: Bitwise operators (cppreference)
    url: https://en.cppreference.com/w/cpp/language/operator_arithmetic#Bitwise_logic_operators
  - title: "Bit Twiddling Hacks (Sean Eron Anderson)"
    url: https://graphics.stanford.edu/~seander/bithacks.html
---
Phase 2 showed you that every object is bytes. This phase goes one
level down, to the bits inside them, and then to the instructions the
CPU runs on them.

**Binary** is positional, like decimal, with 2 as the base. Each digit
is worth twice the one to its right:

```
  1    1    0    0    1    0    1    0
128   64   32   16    8    4    2    1      128 + 64 + 8 + 2 = 202
```

Reading a number from text is the same idea in a loop, for any base:
start at 0, and for each digit do `value = value * base + digit`.

**Hex** is binary in a shorter coat. One hex digit is exactly four
bits, so you can convert in your head a digit at a time: `1100 1010`
is `c` then `a`, `0xca`. That's why memory dumps and addresses are
always hex. C++ lets you write all three, and `'` as a separator
wherever it helps:

```cpp
std::uint32_t a = 202;
std::uint32_t b = 0xca;
std::uint32_t c = 0b1100'1010;   // all the same number
```

The **bitwise operators** work on each bit column separately:

```
a      1100 1010
b      0000 1111
a & b  0000 1010    AND: 1 where both are 1       (keep some bits)
a | b  1100 1111    OR:  1 where either is 1      (switch bits on)
a ^ b  1100 0101    XOR: 1 where they differ      (flip bits)
~a     0011 0101    NOT: every bit flipped
```

Two more move bits sideways: `x << n` shifts left by `n` places, which
multiplies by 2ⁿ, and `x >> n` shifts right. So `1u << 5` is a number
with only bit 5 set, and `(x >> 5) & 1` reads bit 5 of `x`. Bits are
numbered from 0 on the right.

Don't confuse `&` and `|` with `&&` and `||`. The doubled ones work on
whole values as true or false, and stop early.

`main.cpp` is done. It prints two numbers and what the operators make
of them:

```
$ ./bits 202 0x0f
a     = 0000 0000 0000 0000 0000 0000 1100 1010  0x000000ca  202
b     = 0000 0000 0000 0000 0000 0000 0000 1111  0x0000000f  15
a & b = 0000 0000 0000 0000 0000 0000 0000 1010  0x0000000a  10
a | b = 0000 0000 0000 0000 0000 0000 1100 1111  0x000000cf  207
a ^ b = 0000 0000 0000 0000 0000 0000 1100 0101  0x000000c5  197
~a    = 1111 1111 1111 1111 1111 1111 0011 0101  0xffffff35  4294967093
a has 4 one bits and is not a power of two
```

Write these by hand, with the operators above. `std::bitset`,
`std::format("{:b}")`, `std::popcount` and `std::stoul` would do the
work for you, so leave them out this once.

## Assignment

- [ ] `to_binary` prints all 32 bits, highest first, in groups of four
- [ ] `parse_number` reads decimal, `0x` hex and `0b` binary, and rejects anything else
- [ ] `count_ones` counts the 1 bits
- [ ] `is_power_of_two` is true for 1, 2, 4, 8, … and nothing else
