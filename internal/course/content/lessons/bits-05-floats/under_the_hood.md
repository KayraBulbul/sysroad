**A `double` is the same thing, bigger.** 1 sign bit, 11 exponent bits
stored plus 1023, and 52 fraction bits, so 53 bits of precision.
That's every whole number up to 2^53, about 9 × 10^15, and roughly 16
decimal digits. JavaScript has only doubles, which is why its
"safe integer" limit is 2^53 - 1.

**Why the fields are in that order.** With the sign aside, a bigger
exponent always means a bigger number, and within one exponent a
bigger fraction does. So positive floats sort in the same order as
their bit patterns read as integers. Adding 1 to the bits gives the
next float up, and that even steps correctly from the largest
subnormal to the smallest normal, and from the largest normal to
infinity.

**Why subnormals exist.** Without them, the smallest positive number
would be 2^-126 and the next one down would be 0, a sudden gap. Then
`a - b` could be 0 when `a != b`. Subnormals guarantee that `a == b`
exactly when `a - b == 0`. They cost a lot on some CPUs, where an
operation on a subnormal traps to slow microcode, so audio code often
turns them off.

**NaN has payload.** Any non-zero fraction with exponent 255 is a NaN,
so there are 2^24 - 2 different float NaNs. Some JavaScript engines
and LuaJIT use the spare bits of a double NaN to store pointers and
integers, a trick called NaN boxing.

**Ties to even isn't the only rounding.** IEEE 754 defines four modes:
to nearest (the default), toward zero, toward +infinity and toward
-infinity. Ties go to even so that rounding errors don't all push the
same way: always rounding .5 up would make a long sum drift upward.

**The compiler does `from_uint` in one instruction.** `float(n)`
compiles to `cvtsi2ss` on x86-64 and `ucvtf` on ARM. Rounding an
integer into 24 bits is exactly what you wrote, in hardware.
