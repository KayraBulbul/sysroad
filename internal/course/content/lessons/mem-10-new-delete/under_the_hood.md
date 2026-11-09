**Where heap memory comes from.** `malloc` carves blocks out of big
regions it gets from the kernel: `brk` grows the classic heap segment,
and `mmap` maps fresh pages (glibc uses it for blocks over 128 KB).
Phase 7 has you write your own `malloc` on top of `mmap`.

**Freed memory isn't given back right away.** `free` puts the block on
a free list for the next `malloc` of a similar size. That's why a
use-after-free often "works": the bytes are still there, until the next
allocation reuses them. ASan delays reuse on purpose (its *quarantine*)
so it can catch you.

**Vector growth.** A vector that's full allocates a bigger buffer,
twice the size in both libstdc++ and libc++, moves its elements across
and frees the old one. Doubling means `n` push_backs make about
`log2(n)` allocations, so the cost per push_back is constant on
average ("amortised O(1)"). `reserve` lets you skip all of that when
you know the size.

**Small strings don't allocate.** A `std::string` keeps short text
inside the object itself: up to 15 characters in libstdc++ and 22 in
libc++. That's the *small-string optimisation*, and you'll build one in
this phase's last chapter.

**Replacing operator new is allowed.** The standard lets a program
define its own global `operator new` and `operator delete`, and the
linker uses them in place of the library's. That's the hook used by
leak checkers, profilers and allocators like jemalloc and mimalloc.
