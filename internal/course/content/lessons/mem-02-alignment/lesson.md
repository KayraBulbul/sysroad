---
title: Alignment and padding
minutes: 20
files: [layout.cpp]
run:
  args: ["char", "int", "char", "double"]
deeper:
  - title: Objects and alignment (cppreference)
    url: https://en.cppreference.com/w/cpp/language/object#Alignment
  - title: The Lost Art of Structure Packing (Eric S. Raymond)
    url: http://www.catb.org/esr/structure-packing/
---
CPUs load memory in aligned chunks. A 4-byte `int` is cheapest to read
from an address that's a multiple of 4, and on some CPUs a misaligned
load traps. So every type has an **alignment**, `alignof(T)`, and the
compiler only ever places a `T` at an address that's a multiple of it.

Inside a struct, that means gaps:

```cpp
struct S {
    char c;    // offset 0
               // 3 bytes of padding
    int n;     // offset 4: a multiple of alignof(int) == 4
    char d;    // offset 8
               // 3 bytes of padding, so arrays of S keep n aligned
};             // sizeof(S) == 12, alignof(S) == 4
```

The rules the compiler follows:

1. Each field goes at the next offset that's a multiple of its alignment.
2. The struct's alignment is the largest field alignment.
3. The size is rounded up to a multiple of the struct's alignment. That
   tail padding makes `S arr[2]` work: `arr[1].n` must be aligned too.

Alignments are always powers of two, so rounding up has a classic
bit trick: `(n + align - 1) & ~(align - 1)`. Adding `align - 1` pushes
`n` past the next boundary unless it's already on one, and the mask
clears the low bits to land exactly on it.

`main.cpp` is done. It turns type names into fields and prints your
layout:

```
$ ./layout char int char double
offset  size  field
     0     1  char
     1        (3 bytes padding)
     4     4  int
     8     1  char
     9        (7 bytes padding)
    16     8  double
sizeof 24, alignof 8, 10 bytes of padding
```

## Assignment

- [ ] `align_up` rounds `n` up to a multiple of `align`
- [ ] `lay_out` gives every field its offset and the padding before it
- [ ] It works out the struct's size, alignment and tail padding
