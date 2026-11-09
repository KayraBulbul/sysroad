**Why signed overflow is UB at all.** It lets the compiler treat `int`
like maths: `i + 1 > i` is always true, and a loop `for (int i = 0; i < n; ++i)`
runs exactly `n` times, so it can be vectorised and use 64-bit
induction variables. Unsigned arithmetic is defined to wrap, and loses
some of those optimisations.

**Checking for overflow without UB.** `__builtin_add_overflow(a, b, &out)`
(clang and GCC) returns whether it overflowed and is a single
instruction plus a flag check. C++26 adds `std::add_sat` and friends.
Never write `if (a + b < a)` with signed types: that's the code the
optimiser deletes.

**Strict aliasing.** Reading an object through a pointer to an
unrelated type, like a `float` through a `std::uint32_t*`, is UB even
when it's aligned. The compiler assumes pointers of different types
don't point at the same memory and reorders loads and stores
accordingly. `memcpy` and `std::bit_cast` are the legal ways to
reinterpret bytes, and UBSan can't detect aliasing violations. Code
review and `-fno-strict-aliasing` (which the Linux kernel uses) are the
other defences.

**What sanitizers can't see.** Uninitialised reads need
MemorySanitizer, which is Linux only and needs every library rebuilt.
Data races need ThreadSanitizer (phase 8). Some UB has no checker at
all. Warnings help: `-Wall -Wextra` catches many uninitialised
variables at compile time, which is why your builds show them.

**Undefined vs unspecified vs implementation-defined.** *Unspecified*
means one of several valid outcomes, like the order function arguments
are evaluated in. *Implementation-defined* means the compiler picks and
documents it, like `sizeof(long)`. Only *undefined* removes every
guarantee.
