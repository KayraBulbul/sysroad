Go one line at a time and write down what each register holds, in
terms of the arguments. For `second` on x86-64:

```
movq  %rdi, %rax      rax = a
addq  %rsi, %rsi      rsi = b + b
subq  %rsi, %rax      rax = a - 2*b
```

The value in `rax` (or `x0`) at the `ret` is what the function
returns.
---
`third` and `fifth` multiply without a multiply instruction.
`leaq (%rdi,%rdi,4), %rax` is `a + a*4`, which is `5*a`. On arm64,
`add x8, x0, x0, lsl #2` is the same thing: `a + (a << 2)`.

`fifth` uses the 32-bit names (`edi`, `eax`, `w0`) because its
arguments are `int`s. The arithmetic reads the same way.
---
In `sixth`, a left shift by 4 is a multiplication by 16 and a right
shift by 2 is a division by 4, rounding down. Write them as shifts,
`a << 4` and `b >> 2`: for a negative `b`, `b >> 2` and `b / 4` give
different answers, and the tests check negative numbers.
---
The answers have this shape, with the numbers left for you:
`first` is a sum, `second` is `a - ?*b`, `third` is `?*a + ?`,
`fourth` is a difference times `c`, `fifth` is `?*a - b + ?`, and
`sixth` is the sum of two shifts.
