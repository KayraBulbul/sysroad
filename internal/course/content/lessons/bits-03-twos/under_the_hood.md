**The CPU keeps score.** After every `add`, an x86-64 CPU sets two
flags: *carry* (the unsigned result didn't fit) and *overflow* (the
signed result didn't fit). ARM has the same pair, C and V. C++ gives
you no way to read them, so `__builtin_add_overflow(a, b, &sum)` in
clang and gcc exists to do exactly that: one `add`, then a jump on the
overflow flag. C++26 adds `std::add_sat` and friends to the standard.

**Why signed overflow is undefined rather than wrapping.** Partly
history, since not every old machine was two's complement. Today it's
about speed. If `i + 1 > i` is always true, the compiler can turn
`for (int i = 0; i <= n; ++i)` into a loop it knows the trip count
of, and use a 64-bit register for `i` without re-checking for wrap.
If you want wrapping, say so with unsigned types, or build with
`-fwrapv`.

**`>>` on a negative number.** Shifting a signed value right copies
the sign bit in from the left (an *arithmetic* shift), so `-56 >> 1`
is -28. Shifting an unsigned one brings in zeros (a *logical* shift).
The CPU has a separate instruction for each: `sar` and `shr` on
x86-64, `asr` and `lsr` on ARM. Which one you get depends only on the
type.

**Saturation is real.** Audio and image code saturate on purpose: a
too-loud sample should clip at the maximum and a too-bright pixel
should stay white. Wrapping would turn it into a loud click or a black
dot. SIMD instruction sets have saturating adds built in for exactly
this.

**Famous overflows.** Ariane 5 was lost in 1996 when a 64-bit float
was converted to a 16-bit signed integer that couldn't hold it. The
"year 2038 problem" is a signed 32-bit count of seconds since 1970
running out. And binary search was broken in most textbooks for
decades because `(lo + hi) / 2` overflows on big arrays, which is the
`midpoint` bug you fixed in 2.3.6.
