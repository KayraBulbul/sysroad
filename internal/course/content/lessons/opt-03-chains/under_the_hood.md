**Reading your numbers.** On an i9-14900K, `sum1` runs at 2 CPE: the
performance cores add doubles with a latency of 2 cycles. `sum2`
halves it and `sum4` halves it again to 0.5, which is the throughput
bound: two adds per cycle. More accumulators can't beat that, so
`sum8` would gain nothing. Horner's rule is about 7 CPE, a multiply
(4 cycles) then an add (2) on every step, and splitting it halves
that.

**Apple Silicon is different.** Its FP add has 3 cycles of latency,
but the performance cores have four FP units. To keep four adds
busy for three cycles each, you need about 12 chains, so `sum4`
gets 0.75 CPE and more accumulators keep helping. The same code tuned
on one CPU is not tuned for another. Also, arm64 always has fused
multiply-add, and clang fuses `a[i] + x * result` into one `fmadd`
there (4 cycles), so Horner's rule costs less on the Mac.

**Little's law.** The number of operations you need in flight is
latency × throughput. 2 cycles × 2 per cycle = 4 chains on the PC,
3 × 4 = 12 on the Mac. The same rule sizes how many outstanding
cache misses a loop needs to use all the memory bandwidth (lesson
5.1.3).

**Why not `std::array<double, K>` for K chains?** Try it with K = 8:
clang's *SLP vectoriser* tries to pack neighbouring accumulators into
SIMD registers and makes a mess of it, shuffling values every
iteration. Named variables avoid it. Compilers are good, not perfect,
and checking the assembly (or the CPE) is how you find out.

**What `-ffast-math` would do.** With `-ffast-math`, or just
`-fassociative-math`, clang may reassociate floating point itself. It
then turns `sum1` into a vectorised loop with several vector
accumulators, beating `sum4`. You'd get a slightly different answer,
which is fine for a game and wrong for a bank.

**Integer sums are already fast.** Change `double` to `long` and
`sum1` drops well below 1 CPE without any work from you: integer
addition is associative, so clang unrolls, splits and vectorises the
loop by itself.

**The reorder buffer.** Out-of-order execution has limits. The core
tracks a window of a few hundred instructions (512 on Golden Cove,
over 600 on Apple's big cores). Independent work further ahead than
that can't be started early. Long dependency chains fill the window
with instructions that are all waiting.
