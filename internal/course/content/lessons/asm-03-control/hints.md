`alpha`, x86-64: `rax` starts as `b`. `cmpq %rsi, %rdi` then
`cmovgq %rdi, %rax` replaces it with `a` when `a > b`. On arm64 it's
one `csel`. Which of the two arguments does it return?
---
`bravo` compares `x` with 89, then 74, then 50 (or 49). "Greater than
89" is `>= 90` for an `int`. Each comparison that succeeds returns a
different small constant. The last one on x86-64 uses `setge`, which
gives 1 or 0, and then doubles it.
---
`charlie`: the result starts at 1. Each trip round the loop multiplies
it by `a` and takes 1 off `n`, until `n` is 0. What is 1 multiplied
by `a`, `n` times?
---
`delta` counts in `rax` (`x8` on arm64) how many times the loop runs
until `n` is 1. Each time it works out *two* candidates for the next
`n`, `n >> 1` and `3*n + 1`, then tests the lowest bit of the old `n`
to pick one. The lowest bit tells you whether a number is odd.
---
`echo`: on x86-64, `divq %rcx` divides the 128-bit number in
`rdx:rax` by `rcx`. The quotient lands in `rax` and the **remainder**
in `rdx`. The `xorl %edx, %edx` before it clears the top half. On
arm64, `udiv` gives the quotient and `msub x1, x9, x1, x0` computes
`x0 - x9*x1`, which is the remainder.

So each trip does `r = a % b; a = b; b = r;` until `b` is 0. That's
a 2300-year-old algorithm.
