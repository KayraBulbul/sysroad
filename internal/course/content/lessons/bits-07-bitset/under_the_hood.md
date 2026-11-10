**`i / 64` and `i % 64` cost nothing.** For an unsigned `i` the
compiler turns them into `i >> 6` and `i & 63`, a shift and a mask.
That only works because 64 is a power of two, which is one reason
sizes in systems code so often are.

**`std::vector<bool>` is already a bitset.** The standard made it a
special case that packs bits, and it's widely seen as a mistake:
`v[i]` can't return a real `bool&`, so it returns a proxy object, and
generic code that works on every other vector breaks on it. It also
has no whole-set `&`, `|` or `count`. `std::bitset<N>` has those, but
its size is fixed at compile time. Your class fills the gap, as
`boost::dynamic_bitset` does.

**Why the sieve likes this.** Sieving the numbers below a billion
needs a billion flags. As bytes that's 1 GB. As bits it's 125 MB, and
serious sieves store only the odd numbers and work on one
cache-sized block at a time. Lesson 5.1.2 explains why the block
size matters so much.

**Finding the lowest set bit is one instruction.** `std::countr_zero`
is `tzcnt` (or `bsf`) on x86-64 and `rbit` + `clz` on ARM. An
allocator looking for a free slot, or a scheduler looking for the
highest-priority runnable task, uses exactly your `find_first`: skip
whole empty words, then one instruction for the bit.

**Chess engines call these bitboards.** A chessboard has 64 squares,
so "all the white pawns" is one `std::uint64_t`. "Pawns that can
capture to the left" is a shift and an AND over the whole board at
once, with no loop.

**Bloom filters.** A set that answers "definitely not here" or
"probably here" using a bitset and a few hash functions: adding a key
sets a few bits and looking one up tests them. Databases use them to
skip reading files that can't contain a key. You have every piece
needed to write one now: `Bitset` here and hashing from lesson 2.4.4.
