**Why the branchless filter loses on sorted data.** When the predictor
is always right, the branchy loop skips the store for half the
elements and runs at full speed. The branchless one does a store
every time, and its `count` is a dependency chain: each store's
address waits for the previous comparison. Branchless code costs
about the same whatever the data; branchy code is faster when the
data is predictable and much slower when it isn't. Pick by what your
real data looks like.

**What clang did to the `?:`.** x86-64 compilers carry a pass that
turns conditional moves back into branches inside loops when the
result feeds a load address, because a correct prediction lets the
next load start early, while a `cmov` makes it wait for the
comparison. That's a good bet for predictable branches and a bad one
for binary search. `__builtin_unpredictable` switches it off for that
condition. On arm64 there's no such pass, and the `csel` stays.

**How predictors work.** Modern predictors (TAGE and its relatives)
keep tables indexed by the branch's address combined with the
outcomes of the last few dozen to few thousand branches, so they
learn patterns that span loops and function calls. They're right well
over 95% of the time on typical code. The misses that remain are data
dependent, like yours.

**The 15-20 cycles.** A misprediction is found when the branch
finally executes, deep in the pipeline. Everything fetched after it
has to be thrown away, and fetching restarts at the right address.
Measured in lost work, it's even worse: a core that could have
retired 4-8 instructions per cycle retired none for those cycles.

**Sorting first.** Sorting the data makes the branchy filter fast,
but costs far more than the filter does. It's the right trade only
when the data is reused many times. Databases do exactly that:
sorted or clustered columns make their scans predictable.

**Spectre.** Speculation is supposed to be invisible, because wrong
work is thrown away. In 2018 the Spectre attacks showed that it
leaves traces in the cache, and that a mistrained predictor can make
a victim speculatively read memory it shouldn't. That's why browsers
and kernels now limit timer precision and insert speculation barriers
in places.
