---
title: "Arrays, structs and pointers in assembly"
minutes: 35
files: [item.hpp, mystery.cpp]
deeper:
  - title: "x86 addressing modes (Wikibooks: x86 Assembly)"
    url: https://en.wikibooks.org/wiki/X86_Assembly/X86_Architecture#Addressing_modes
  - title: "A64 loads, stores and addressing modes (Arm Learn)"
    url: https://developer.arm.com/documentation/102374/0102/Loads-and-stores---addressing
---
Arrays, structs and linked lists don't exist in machine code. There is
only memory, and instructions that read or write a few bytes at an
address. The address is computed from a register or two and some
constants, and those constants are the layout you learned in chapter
2.1: element sizes and field offsets.

**x86-64.** A memory operand is written `D(B,I,S)` and means the
address `B + I*S + D`. Any part can be left out:

```
movq  (%rdi), %rax            rax = *p
movq  16(%rdi), %rax          rax = the 8 bytes at p + 16: a field, or p[2]
movq  (%rdi,%rcx,8), %rax     rax = p[i]      for 8-byte elements
movl  8(%rdi,%rsi,4), %eax    eax = p[i + 2]  for 4-byte elements
movq  %rax, (%rdi)            *p = rax        memory on the right is a store
addq  %rax, (%rdi)            *p += rax       load, add and store in one
```

It's the same syntax `leaq` uses, but `leaq` only computes the
address, and these go to memory.

**arm64.** Only `ldr` (load) and `str` (store) touch memory, and the
address is in square brackets:

```
ldr   x8, [x0]                x8 = *p
ldr   x8, [x0, #16]           x8 = the 8 bytes at p + 16
ldr   x8, [x0, x1, lsl #3]    x8 = p[i]       x1 shifted by 3 is i * 8
ldr   x9, [x0], #8            x9 = *p, then p += 8    ("post-index")
str   x8, [x0]                *p = x8
```

**The instruction tells you the size and the type.** A `long` or a
pointer is 8 bytes, an `int` 4, a `short` 2 and a `char` 1, and
smaller values are widened as they're loaded:

- x86-64: `movq` loads 8 bytes and `movl` 4. `movslq` loads an `int`
  and sign-extends it to 64 bits. `movswl` loads a `short`, `movsbl`
  a `char`, and `movzbl` an unsigned byte.
- arm64: `ldr` into an `x` register loads 8 bytes, into a `w`
  register 4. `ldrsw` loads an `int` as a 64-bit value, `ldrsh` a
  `short`, `ldrsb` a `char`, and `ldrb` an unsigned byte. `ldur` is
  `ldr` with an offset that `ldr` can't encode, such as a negative one.

**How to read the shapes.**

- An **array loop** either indexes, with a counter multiplied by the
  element size, or bumps the pointer by the element size on each
  trip. Either way that number is `sizeof` one element.
- A **struct field** is a fixed offset from a pointer. Names are
  gone, so you only see `8(%rdi)` and have to work out which field
  lives at 8.
- A **linked list** is a loop where the pointer is replaced by
  something loaded through itself: `movq 16(%rdi), %rdi` is
  `node = node->next`.

The listings have six functions. Two of them, `november` and `oscar`,
take an `Item`, and `mystery.hpp` tells you in words what they do. The
`Item` in `item.hpp` has the right fields in the wrong order. The
tests build items from raw bytes, as if read from a file, so your
struct has to have the layout the compiled code expects.

## Assignment

- [ ] `kilo`, `lima` and `mike`: arrays and pointers
- [ ] Put `Item`'s fields in the order the offsets in `november` and `oscar` show
- [ ] `november` and `oscar`: written from their descriptions, using the fields by name
- [ ] `papa`: a walk along a linked list of `Node`s
