**Why 1024 is slower than 1000.** A row of 1024 doubles is 8 KB, so
walking down a column lands on addresses exactly 8 KB apart. The set
index comes from the address bits just above the line offset, and
8 KB apart means the same bits: every element of the column goes to
the same set. With 12 ways, the 13th row evicts the first, and a
column walk misses on every read. With 1000 columns the rows are 8000
bytes apart and spread over all the sets. The same thing makes the
textbook multiply of 512 × 512 three times slower than 500 × 500,
even though it does only 7% more work. It's the conflict miss from
lesson 5.1.2 on real hardware.

**Picking a block size.** The tile has to fit in L1 with room to
spare, and for doubles a tile row of 8 fills exactly one 64-byte line.
Too big, and the tile's rows start evicting each other again, which
is why 16 or 32 helps less at 1024. Libraries pick block sizes per
CPU, or tune them by measuring. *Cache-oblivious* algorithms instead
split the matrix in half recursively, so some level of the recursion
fits each cache without knowing its size.

**The other loop orders.** There are six orders for the three loops.
`ikj` and `kij` walk rows of both `b` and `c` in the inner loop.
`ijk` and `jik` walk a column of `b`. `jki` and `kji` walk columns of
both `a` and `c`, and are the slowest. CS:APP's figure 6.44 measures
all six.

**Vectorising comes free with the right order.** The `ikj` inner loop,
`c(i, j) += x * b(k, j)` over `j`, touches consecutive elements with
no dependency between iterations, so `-O2` turns it into SIMD
instructions that do 2, 4 or 8 at once. The `ijk` inner loop is a dot
product into one `sum`, a dependency chain that clang won't reorder
for floating point without `-ffast-math`. Some of your speed-up is
that, not the cache. Lessons 5.2.3 and 5.2.4 pull the two apart.

**What real libraries do.** An optimised BLAS (OpenBLAS, MKL, Apple's
Accelerate) blocks for every level of cache and for the registers,
packs tiles into contiguous buffers, and uses hand-written SIMD
kernels. It runs 50-100× faster than `ijk` on big matrices, and it's
why NumPy's `@` is fast.
