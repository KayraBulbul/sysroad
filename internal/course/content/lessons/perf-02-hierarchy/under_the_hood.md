**The column walk.** A row of 512 ints is 2048 bytes, which is 32
lines. Walking down a column, every step lands 32 lines further on, so
`set_of` only ever gives two different sets: 64 sets divided by a
stride of 32. 512 lines fight over 16 ways and every read misses, even
though the 512 lines would fill the cache exactly. Padding each row by
one line makes the stride 33 lines, which visits every set, and the
column walk does as well as the row walk. That's a conflict miss and
its cure. Libraries like Eigen pad matrices for this reason, and the
same thing bites 4 KB-aligned buffers and power-of-two hash tables.

**In hardware it's bits.** Because everything is a power of two, the
offset, set and tag are just bit ranges: for 64-byte lines and 64
sets, bits 0-5 are the offset, 6-11 the set and the rest the tag. The
hardware checks all the ways of a set in parallel, which is why L1s
stop at 8 or 12 ways: more comparators cost power and time.

**LRU is an approximation.** Tracking the exact order of 8 or 16 ways
costs too many bits, so real caches use cheaper schemes ("tree
pseudo-LRU", or ones that guess which lines will be reused). The
simulator's answers are close to real hardware, not identical.

**What the simulator leaves out.** Real CPUs **prefetch**: when they
spot you reading lines in order, or at a fixed stride, they fetch the
next ones before you ask. That turns most of the "1 MB in order"
misses into hits, and it's why sequential access is even faster than
the miss count suggests. Writes are cached too: a written line is
marked *dirty* and only goes back to memory when it's evicted
(*write-back*).

**Physical addresses and the TLB.** L1 caches are indexed by virtual
address bits that are the same in the physical address, which is why
L1 size is usually ways × 4 KB: 8 ways × 4 KB = 32 KB, 12 × 4 KB =
48 KB. Apple's 16 KB pages let its L1 be bigger. Translating virtual
to physical addresses has its own cache, the TLB, and a TLB miss
costs a walk through the page tables. Big strides through big arrays
pay that too.

**CS:APP's Cache Lab** has you write this simulator in C against
valgrind traces, then optimise a matrix transpose for the fewest
misses. Lesson 5.1.4 does the second part with your simulator.
