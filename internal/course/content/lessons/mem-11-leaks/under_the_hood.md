**How ASan knows.** ASan keeps *shadow memory*: one byte of shadow for
every 8 bytes of your memory, saying which bytes are valid to touch.
The compiler adds a shadow check before every load and store. Freed
blocks are *poisoned* and held in a quarantine instead of being reused
straight away, so a later access to them is caught rather than landing
in some new allocation.

**How LeakSanitizer knows.** At exit it pauses the program and scans
everything still reachable (globals, stacks, registers) for pointers
into heap blocks, like a garbage collector's mark phase. Blocks nothing
points to are *direct leaks*. Blocks only reachable from leaked blocks
are *indirect leaks*. Fix the direct ones and the indirect ones go away
with them.

**Why the mismatched delete crashes.** For a type with a destructor,
`new T[n]` allocates a few extra bytes in front of the array to store
`n` (the *array cookie*), so `delete[]` knows how many destructors to
run. It returns a pointer just past the cookie. Plain `delete` hands
that pointer to the allocator, which never gave it out. That's the
`attempting free on address which was not malloc()-ed` report.

**Valgrind** finds the same bugs without recompiling, by running your
program on a simulated CPU, about 20–50× slower. ASan needs a rebuild
but costs about 2×. Use Valgrind for binaries you can't rebuild.

**The real fix is ownership.** Every bug here is a raw pointer with no
clear owner. The rest of this chapter replaces them with types whose
destructors free things exactly once: `std::vector`, `std::string` and,
next, `std::unique_ptr`.
