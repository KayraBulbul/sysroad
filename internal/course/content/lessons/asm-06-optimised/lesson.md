---
title: What -O2 does to your code
minutes: 30
files: [mystery.cpp]
deeper:
  - title: "What has my compiler done for me lately? (Matt Godbolt, CppCon 2017)"
    url: https://www.youtube.com/watch?v=bSkpMdDe4g4
  - title: "Division by invariant integers using multiplication (Granlund & Montgomery)"
    url: https://gmplib.org/~tege/divcnst-pldi94.pdf
  - title: "Hacker's Delight magic number calculator"
    url: https://www.hackersdelight.org/magic.htm
---
So far the listings were built with `-O1`, which stays close to what
you wrote. Real programs ship at `-O2`, where the compiler's job is to
produce the same *results* by any means it can find. Here is one
function both ways:

```cpp
static long cube(long x) { return x * x * x; }

long cubes() {
    long total = 0;
    for (long i = 1; i <= 4; ++i) {
        total += cube(i);
    }
    return total;
}
```

```
cubes:                           x86-64, -O0
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $16, %rsp
    movq    $0, -8(%rbp)         total = 0, in memory
    movq    $1, -16(%rbp)        i = 1, in memory
.LBB1_1:
    cmpq    $4, -16(%rbp)
    jg      .LBB1_4
    movq    -16(%rbp), %rdi
    callq   _ZL4cubel            a real call to cube
    addq    -8(%rbp), %rax
    movq    %rax, -8(%rbp)
    movq    -16(%rbp), %rax
    addq    $1, %rax
    movq    %rax, -16(%rbp)
    jmp     .LBB1_1
.LBB1_4:
    movq    -8(%rbp), %rax
    addq    $16, %rsp
    popq    %rbp
    retq
```

```
cubes:                           x86-64, -O2
    movl    $100, %eax
    retq
```

The compiler **inlined** `cube` (pasted its body into the loop),
noticed that every input is known, and ran the whole loop itself.
1 + 8 + 27 + 64 is 100. On arm64 it's `mov w0, #100` and `ret`.

When the inputs aren't known, it rewrites slow operations as fast
ones. These are the patterns you'll meet most, and each one looks
like nonsense until you know it:

- **Multiplying by a constant** becomes shifts, adds and `lea`:
  `x * 9` is `leaq (%rdi,%rdi,8), %rax`, and `x * 7` is "shift left
  by 3, subtract `x`".
- **Dividing an unsigned number by a power of two** is a right shift,
  and the remainder is an AND: `x / 8` is `shrl $3`, `x % 8` is
  `andl $7`.
- **Dividing a signed number by a power of two** is a shift with a
  correction. `sar` rounds down, and C++ division rounds toward zero,
  so for negative numbers the compiler adds `divisor - 1` first.
- **Dividing by any other constant** becomes a multiplication by a
  huge "magic" number followed by a right shift. `x / 3` for an
  unsigned `x` is `(x * 2863311531) >> 33`, because 2863311531 is
  2^33 / 3 rounded up. To find the divisor, compute
  2^shift / magic and round: here 8589934592 / 2863311531 = 2.9999999997,
  so the divisor is 3.
- **A range check** becomes one subtraction and one *unsigned*
  compare: `x >= 5 && x <= 8` is `x - 5 < 4`. If `x` is below 5, the
  subtraction wraps around to a huge number and fails the test too.
- **A `switch`** that returns constants becomes a table: check the
  range, then load `table[x]`. The table is data at the bottom of
  the listing.
- **A call at the very end** becomes a jump: `jmp helper` instead of
  `callq helper` and `retq`. That's the tail call from lesson 4.2.2.

The listings have five functions, built with `-O2`. Each one was a
single plain line of C++ before the compiler got to it, apart from
the last, which was a `switch`. Write that plain version. You
don't need to copy the compiler's tricks: it will apply them to your
code again.

## Assignment

- [ ] `quebec`: a multiplication by a magic number
- [ ] `romeo`: two `lea`s, or a shift and a subtract
- [ ] `sierra`: an add, a conditional move and a shift
- [ ] `tango`: a subtract and an unsigned compare
- [ ] `uniform`: a table lookup
