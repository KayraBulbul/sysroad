**Why only bytes?** C++'s *strict aliasing* rule says an `int*` and a
`float*` can't point at the same object, and neither can a `long*`
and a `double*`. The compiler uses that freely: a store through a
`float*` can't change `img.size`, a `std::size_t`. The exceptions are
`char`, `unsigned char` and `std::byte`, which exist to let you look
at the raw bytes of anything (that's how `memcpy` and your hexdump
work). So byte buffers, the most common data in systems code, are the
one place where every store might alias everything. Change `pixels`
to a `float*` and the original `invert` vectorises fine.

**When pointers of the same type might overlap,** clang often emits
two versions of the loop: a fast vectorised one, and a plain one, plus
a check at the start of whether the arrays overlap. That *loop
versioning* is why the textbook examples, like summing a row of `a`
into `b[i]`, run just as fast in clang today as the "fixed" versions.
It only works when the compiler can work out the address ranges up
front. When the bounds themselves might change, as here, it can't.

**`__restrict`.** C99's `restrict` keyword promises the compiler that
a pointer is the only way to reach its memory. C++ doesn't have it,
but clang and GCC accept `__restrict` as an extension:
`void f(std::uint8_t* __restrict p, …)`. Break the promise and the
behaviour is undefined. Locals are the safer fix, and usually as
fast.

**Other blockers.** A call to a function in another `.cpp` file (no
LTO) might read or write any global and anything reachable from its
arguments, so values have to go back to memory before the call and
be reloaded after it. `volatile` forbids keeping a value in a
register at all. Floating-point sums can't be reordered without
`-ffast-math`. And exceptions: a call that might throw needs the
loop's state to be consistent at that point.

**How to find them.** `-Rpass-missed=loop-vectorize` names every loop
clang didn't vectorise, and `-Rpass-analysis=loop-vectorize` says why,
for example "cannot identify array bounds" or "loop not vectorized:
cannot prove it is safe to reorder memory operations". Lesson 5.2.4
uses them.

**`std::strlen` is fast, and that's the trap.** It reads 16 or 32
bytes per instruction, so for a 100-byte string the quadratic loop
looks fine. At 100 KB it does 5 billion bytes of work. The same
pattern hides in any loop condition that searches, like
`i < s.find(…)`.
