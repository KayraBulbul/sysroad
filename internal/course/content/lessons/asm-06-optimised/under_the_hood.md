**The optimiser's licence is "as if".** The standard says a program
must *behave as if* it ran the code you wrote. Anything you can't
observe from inside the program is fair game: variables don't have
to exist, loops don't have to loop, functions don't have to be
called. Timing and memory use are not observable in this sense. That
is also where undefined behaviour bites: the compiler assumes it
never happens, and optimises accordingly.

**Why division gets this treatment.** A 64-bit `div` takes 20 to 40
cycles. A multiply takes 3 and a shift 1. Division by a constant is
common (think `% 10` when printing a number), so this one trick pays
for itself constantly. When the divisor is a variable, the compiler
has no choice but to emit `div`.

**Even the loop can go.** At `-O2`, `for (i = 1; i <= n; ++i) sum += i;`
compiles to no loop at all: clang recognises the pattern and emits
the closed form n(n+1)/2 as a few multiplies and shifts. It can't do
that for every loop, only for sums of polynomials in the counter.

**Vectorising.** When a loop does the same thing to every element of
an array, `-O2` rewrites it to handle 4, 8 or 16 elements per trip
using the SIMD registers (`xmm`/`ymm` on x86-64, `v` on arm64). The
listing gets several times longer: a fast loop, a slower loop for the
leftovers, and checks that pick between them. Lesson 5.2.4 covers it.

**Jump tables.** When the cases of a `switch` run code and don't just
return constants, the table holds *addresses* and the code does an
indirect jump: `jmpq *(%rcx,%rax,8)`. The bomb lab has one of these.
Find the table, and each entry tells you where one case starts.

**Don't do the compiler's job in your source.** Writing `x << 3`
when you mean `x * 8`, or keeping a hand-made lookup table for a
simple `switch`, makes the code harder to read and no faster. The
exception is when you've measured. That's phase 5.

**`-O3`, `-Os`, `-Og`.** `-O3` adds more aggressive loop
transformations and is sometimes slower because the code gets bigger.
`-Os` optimises for size. `-Og` is meant for debugging, and in clang
it's the same as `-O1`.
