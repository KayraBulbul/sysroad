`foxtrot`: on x86-64 the arguments are `a` to `f` in `rdi`, `rsi`,
`rdx`, `rcx`, `r8`, `r9`, then `g` at `8(%rsp)` and `h` at
`16(%rsp)`. On arm64 they're simply `x0` to `x7`. Which three does
the code touch, and what does it do with them?
---
`golf`: ignore the `pushq %rax` and `popq %rcx`. They don't save
anything useful. They move `rsp` by 8 so that it's a multiple of 16
at the `callq`, which the convention requires. What's left is a call
and one instruction on the result.
---
`hotel`: `b` is parked in `rbx` (or `x19`) before the first call.
The first result is parked in `r14` (or `x20`) before the second.
Then the two results are added.
---
`india` returns 0 straight away when `n <= 0`. Otherwise it keeps `n`
in a surviving register, calls *itself* with `n - 1`, passes that
result to `square`, and adds `n`. Write it exactly like that, as a
recursive function.
---
`juliet` reserves room for a local array and passes its address and
`seed` to `fill`. Afterwards it loads the 8-byte values at offsets 0
and 24 from the start of the array. With 8-byte elements, offset 24
is element 3. So: `long values[4]; fill(values, seed);` and return
the sum of two elements.
