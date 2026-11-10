---
title: Branches and loops
minutes: 30
files: [mystery.cpp]
deeper:
  - title: "x86 condition codes: the full Jcc table (Félix Cloutier)"
    url: https://www.felixcloutier.com/x86/jcc
  - title: "ARM condition codes (Arm developer documentation)"
    url: https://developer.arm.com/documentation/dui0801/l/Condition-Codes/Condition-code-suffixes-and-related-flags
---
A CPU has no `if`, `while` or `for`. It has two things: an instruction
that **compares** and remembers the outcome in a few *flag* bits, and
instructions that **jump** to a label, or don't, depending on those
flags. Every control structure you know is built from those.

```cpp
long factorial(long n) {
    long result = 1;
    while (n > 1) {
        result *= n;
        --n;
    }
    return result;
}
```

```
factorial:                       x86-64
    movl    $1, %eax             result = 1
    cmpq    $2, %rdi             compare n with 2
    jl      .LBB0_2              if n < 2, skip the loop
.LBB0_1:
    imulq   %rdi, %rax           result *= n
    cmpq    $2, %rdi             compare n with 2
    leaq    -1(%rdi), %rdi       n = n - 1   (lea doesn't touch the flags)
    jg      .LBB0_1              if n was > 2, go round again
.LBB0_2:
    retq
```

```
_factorial:                      arm64
    cmp     x0, #2
    b.lt    LBB0_4               if n < 2, skip the loop
    mov     x8, x0               x8 = n
    mov     w0, #1               result = 1
LBB0_2:
    mul     x0, x0, x8           result *= n
    cmp     x8, #2
    sub     x8, x8, #1           n = n - 1
    b.gt    LBB0_2               if n was > 2, go round again
    ret
LBB0_4:
    mov     w0, #1
    ret
```

Three things to take from it:

- **A jump backwards is a loop.** The condition is tested once before
  the loop, to skip it entirely, and again at the bottom.
- **A jump forwards is an `if`**, skipping the code it guards.
- **The compiler rewrites conditions.** `n > 1` became "not less than
  2". Don't expect your constants back unchanged.

**x86-64.** `cmpq B, A` followed by `jg` jumps when `A > B`. Read the
operands right to left, another gift of AT&T syntax. The letters after
`j` name the condition:

- `e` equal, `ne` not equal
- `g`, `ge`, `l`, `le`: greater and less, for **signed** numbers
- `a`, `ae`, `b`, `be`: "above" and "below", for **unsigned** numbers

`testq %rsi, %rsi` compares a register with zero, so `je` after it
means "if `rsi` is 0". The same letters appear on two other
instructions. `cmovgq %rdi, %rax` copies only if the condition holds,
an `if` with no jump. `setge %al` writes 1 or 0 into `al`, the low
byte of `rax`. And `xorl %eax, %eax` is how the compiler writes
`rax = 0`.

**arm64.** `cmp x0, x1` followed by `b.gt` jumps when `x0 > x1`. The
conditions are `eq`, `ne`, then `gt`, `ge`, `lt`, `le` for signed and
`hi`, `hs`, `lo`, `ls` for unsigned. `cbz x1, label` jumps if `x1` is
zero without a `cmp`, and `cbnz` if it isn't. `subs` is a `sub` that
also sets the flags. Conditional select is `csel x0, x1, x2, gt`,
which means `x0 = gt ? x1 : x2`. `csinc` is the same but adds 1 to
the second choice.

Which jump the compiler picks tells you the type: `jg` or `b.gt`
means the source compared signed numbers, `ja` or `b.hi` unsigned.

The listings have five functions. Their names won't help you.

## Assignment

- [ ] `alpha`: a compare and a conditional move
- [ ] `bravo`: a chain of `if`s
- [ ] `charlie`: a loop that counts down
- [ ] `delta`: a loop with an `if` inside
- [ ] `echo`: a loop around a division
