---
title: Registers and your first instructions
minutes: 25
files: [mystery.cpp]
deeper:
  - title: Compiler Explorer (type C++, see the assembly)
    url: https://godbolt.org/
  - title: "x86-64 Machine-Level Programming (CS:APP, chapter 3 as a web aside)"
    url: https://www.cs.cmu.edu/~fp/courses/15213-s07/misc/asm64-handout.pdf
  - title: "A Guide to ARM64 / AArch64 Assembly on Linux (modexp)"
    url: https://modexp.wordpress.com/2018/10/30/arm64-assembly/
---
The compiler turns your C++ into **machine code**: the instructions
the CPU actually runs. **Assembly** is machine code written as text,
one instruction per line. You'll rarely write it. Reading it is the
skill, because it's how you find out what your program really does,
and how you debug one when you have no source.

Your PC and your Mac have different CPUs with different instruction
sets, **x86-64** and **arm64**. The ideas are the same on both, and
every lesson in this chapter shows the two side by side. Learn the one
for the machine you're on first. The labs at the end of the phase are
x86-64.

**Registers** are the CPU's variables. There are only a few, they have
fixed names, each holds 64 bits, and they have no types.

- x86-64 has 16: `rax`, `rbx`, `rcx`, `rdx`, `rsi`, `rdi`, `rbp`,
  `rsp` and `r8` to `r15`. The low 32 bits of each have their own
  name, with an `e`: `eax`, `edi`, and `r8d` for the numbered ones.
- arm64 has 31: `x0` to `x30`, plus `sp`. The low 32 bits of `x0` are
  called `w0`.

A function finds its **arguments** in fixed registers, in order, and
leaves its **result** in another:

- x86-64: arguments in `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`, result in `rax`
- arm64: arguments in `x0` to `x7`, result in `x0`

A `long` or a pointer uses the whole register, and an `int` uses the
32-bit half. Here is one function, compiled for each machine:

```cpp
long blend(long a, long b) { return a + 4 * b + 10; }
```

```
blend:                               x86-64
    leaq    (%rdi,%rsi,4), %rax      rax = rdi + rsi*4
    addq    $10, %rax                rax = rax + 10
    retq                             return; the result is in rax
```

```
_blend:                              arm64
    add     x8, x0, x1, lsl #2       x8 = x0 + (x1 << 2)
    add     x0, x8, #10              x0 = x8 + 10
    ret
```

How to read the x86-64 lines, in the syntax clang and gdb print (it's
called AT&T syntax):

- **The destination is last.** `addq $10, %rax` adds 10 *to* `rax`.
  Most instructions have two operands, and the second is both an input
  and the output: `subq %rsi, %rax` means `rax = rax - rsi`.
- `%` marks a register and `$` a constant.
- The letter on the end is the size: `q` for 64 bits, `l` for 32.
- `movq` copies. `addq`, `subq` and `imulq` do arithmetic. `negq`
  negates. `shlq` shifts left, and `sarq` shifts right keeping the sign.
- `leaq` is the odd one. `(%rdi,%rsi,4)` means `rdi + rsi*4`, and
  `leaq` just computes that sum into the destination. The multiplier
  can be 1, 2, 4 or 8, and a number in front is added too:
  `leaq 7(%rdi,%rdi,2), %rax` is `rax = rdi + rdi*2 + 7`. The compiler
  uses it for arithmetic all the time.

And the arm64 lines:

- **The destination is first**, and the inputs are separate:
  `sub x0, x1, x2` means `x0 = x1 - x2`.
- `#` marks a constant.
- The last operand can be shifted on the way in: `x1, lsl #2` is
  `x1 << 2`, which is `x1 * 4`. `lsl` is a left shift and `asr` a
  right shift that keeps the sign.
- `mov` copies, and `add`, `sub`, `mul` and `neg` do what they say.
- `x8` and up are scratch registers for intermediate values.

Next to `mystery.cpp` are two listings, `x86-64.s` and `arm64.s`. They
are the same six functions, compiled for each machine, and their
source is gone. Work out what each one computes and write it again in
C++.

## Assignment

- [ ] Read the listing for your machine, one function at a time
- [ ] Write each of the six functions in `mystery.cpp` so it returns what the assembly computes
- [ ] Compare with the other machine's listing: the same arithmetic in different words
