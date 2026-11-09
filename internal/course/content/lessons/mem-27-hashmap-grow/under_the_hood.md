**Backward-shift deletion.** Linear probing has a tombstone-free way to
delete. After emptying a slot, walk the run that follows and pull back
any entry that would be findable from the hole. No tombstones means
lookups never slow down. The cost is that `erase` does more work and
is trickier to get right. Robin Hood tables usually do this.

**Iteration order is meaningless.** `for_each` visits slots in table
order, which depends on the hash function, the capacity and the order
of history. libstdc++ and libc++ hash strings differently, so the same
program lists in a different order on Linux and on your Mac. That's why
`main.cpp` sorts before printing. Never let program output depend on
hash order. Go even randomises its map iteration on purpose so nobody
can.

**Rehashing invalidates.** A rehash moves every entry, so pointers from
`find` and references from `operator[]` dangle afterwards, just like a
vector growing. `std::unordered_map` avoids this by chaining (nodes
never move). That's a big part of why it's slower than flat tables:
the standard promised stable references, and that promise costs a heap
allocation per entry.

**Amortised O(1).** Doubling means each entry is moved O(1) times on
average, the same argument as `Vector` growth. The same-size rehash for
tombstones only happens after at least a quarter of the slots have been
used up by erases, so it's amortised too.
