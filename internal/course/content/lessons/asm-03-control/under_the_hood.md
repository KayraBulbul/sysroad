**The flags.** A compare is a subtraction whose result is thrown away.
What's kept is four bits: *zero* (the result was 0), *sign* (it was
negative), *carry* (it borrowed, as unsigned numbers) and *overflow*
(it overflowed, as signed numbers). `je` checks zero. `jb` checks
carry. `jl` checks that sign and overflow differ, which is "less
than" for signed numbers even when the subtraction overflowed. arm64
has the same four, named N, Z, C and V.

**Why branchless code.** A modern CPU starts running the instructions
after a jump before it knows which way the jump goes. It guesses, and
a wrong guess costs around 15 to 20 cycles of thrown-away work.
`cmov` and `csel` have nothing to guess, so the compiler prefers them
when both sides are cheap to compute. `delta` computes both `n >> 1`
and `3*n + 1` every time for that reason. Lesson 5.2.5 measures the
difference.

**Loops get turned upside down.** You wrote the test at the top. The
compiler put one copy before the loop and one at the bottom, so each
trip costs one jump and not two. If you see a test, then a label,
then a body ending in a conditional jump back, it was a `while` or a
`for`. Without the test in front it was a `do … while`.

**`lea` to keep the flags.** In `factorial`, the compiler subtracted 1
with `leaq -1(%rdi), %rdi` and not `decq`. `lea` is the only
arithmetic that leaves the flags alone, so the result of the `cmpq`
above it is still there for the `jg` below.

**Division is the slow one.** `divq` takes 20 to 40 cycles where an
add takes one. It's also awkward on x86-64: it always uses `rax` and
`rdx`, which is why the listing shuffles values around before it.
When the divisor is a constant, the compiler avoids `div` entirely,
as you'll see in lesson 4.2.6.

**Signed or unsigned, read from the jump.** The bits of a register
don't say whether they're signed. The only evidence is which
instructions the compiler chose: `jg`/`jl` or `ja`/`jb`, `sar` or
`shr`, `idiv` or `div`, `movslq` or `movl`. When you rebuild a
function from assembly, those choices give you its types.
