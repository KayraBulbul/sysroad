**`unsigned char` is special.** The language says any object may be
read through `char`, `unsigned char` or `std::byte`, and through nothing
else of the wrong type. That's why `memcpy` works on anything: it copies
bytes. `std::byte` (C++17) is an `unsigned char` that doesn't pretend to
be a number. You can't add to it, only shift and mask.

**Why the mantissa has no leading 1.** A normal float always looks like
`1.xxxx × 2^e`, so the leading 1 isn't stored. That's one free bit of
precision. Exponent 0 means a *subnormal* (no implied 1, for numbers
very close to zero), and exponent 255 means infinity or NaN. `-0.0`
exists and has only the sign bit set. It compares equal to `0.0`, but
`1 / -0.0` is `-inf`.

**Why 0.1 + 0.2 != 0.3.** 0.1 has no exact binary fraction, just like
1/3 in decimal, so it's rounded to the nearest float. The rounding
errors of 0.1 and 0.2 don't cancel. Try `./bytes double 0.1` and look
at the repeating `99 99 99` pattern in the mantissa.

**See it in a debugger.** In gdb, `x/4xb &x` prints the 4 bytes at
`&x` (`x/4xb` = examine 4, hex, bytes). In lldb, use
`memory read -s1 -c4 &x`.
