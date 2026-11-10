`quebec`: the magic number is 3435973837 and the shift is 35. On
arm64 the constant is built in two halves, `52429` and
`52428 << 16`, which is the same number. 2^35 is 34359738368, and
34359738368 / 3435973837 is 9.9999999994. Round it, and that's the
divisor.
---
`romeo` on x86-64: `leaq (%rdi,%rdi,4), %rax` is `x * 5`, and
`leaq (%rax,%rax,2), %rax` multiplies that by 3. On arm64,
`x << 4` minus `x` is `16x - x`. Both are the same constant times
`x`.
---
`sierra` computes `x + 3`, keeps it only if `x` is negative (`cmovns`
copies when the sign bit is *not* set, and arm64's `mi` means
"minus"), then shifts right by 2. That's the signed-division pattern
from the lesson, with a divisor of 2^2. Write it as a division, not
as a shift: `x >> 2` gives a different answer for negative `x`.
---
`tango` subtracts 48 from the character and asks whether the result,
as an unsigned number, is below 10. 48 is `'0'`. So which characters
make it return true?
---
`uniform` subtracts 1 from `month`, returns -1 if that's above 11 as
an unsigned number (so if `month` was below 1 or above 12), and
otherwise loads entry `month - 1` of the table at the bottom. Entry 0
is for month 1. You can write it as a `switch` with twelve cases or
as an array of twelve numbers with a range check in front.
