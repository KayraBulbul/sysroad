---
title: "Two's complement and overflow"
minutes: 20
files: [twos.cpp]
run:
  args: ["8", "200"]
deeper:
  - title: "Two's complement (Wikipedia)"
    url: https://en.wikipedia.org/wiki/Two%27s_complement
  - title: "Signed integers are two's complement (the C++20 paper)"
    url: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0907r4.html
---
Bits have no sign. The same eight bits `1100 1000` are 200 when you
read them as unsigned, and -56 when you read them as signed. The type
decides which.

Signed integers use **two's complement**: the top bit counts as
*negative*, and every other bit keeps its usual value.

```
   1    1    0    0    1    0    0    0
-128   64   32   16    8    4    2    1      -128 + 64 + 8 = -56
```

So for 8 bits:

```
0111 1111    127     the largest
0000 0001      1
0000 0000      0
1111 1111     -1     all ones is always -1
1000 0000   -128     the smallest; there is no +128
```

To negate a number, flip every bit and add 1: `-x == ~x + 1`. The
range is lopsided, -128 to 127, which is why negating the smallest
value can't work.

The reason every CPU uses this scheme: **addition is the same circuit
for signed and unsigned**. `1111 1111 + 0000 0001` overflows to
`0000 0000` whether you call it 255 + 1 or -1 + 1. The CPU has one
`add` instruction and doesn't care what you meant.

When a signed value moves to a wider type, the new high bits are
copies of the sign bit. That's **sign extension**, and it keeps the
value the same: -56 is `1100 1000` in 8 bits and `1111 1111 1100 1000`
in 16.

What happens when a result doesn't fit depends on the type:

- **unsigned** arithmetic wraps around, guaranteed: `0u - 1` is the
  largest value. Converting an out-of-range number to an integer type
  wraps the same way (guaranteed since C++20).
- **signed** overflow is undefined behaviour, as in lesson 2.3.6.
  UBSan stops the program, and without it the optimiser may assume it
  never happens.

So overflow-safe code checks *before* it adds, or does the sum in a
wider type, or adds as unsigned and converts back:

```cpp
if (b > 0 && a > INT32_MAX - b) { /* a + b would overflow */ }
```

`main.cpp` is done. It shows a value at a width, and what happens when
an addition overflows:

```
$ ./twos 8 200
200 in 8 bits is 1100 1000
unsigned 200, signed -56
$ ./twos add 2147483647 1
2147483647 + 1 overflows 32 bits
it wraps to -2147483648 and saturates at 2147483647
```

## Assignment

- [ ] `sign_extend` reads the low `bits` bits as a two's complement number
- [ ] `add_overflows` says whether `a + b` fits, without overflowing itself
- [ ] `wrapping_add` gives what the CPU's `add` would
- [ ] `saturating_add` stops at the largest or smallest value instead
- [ ] `checked_mul` returns the product only when it fits
