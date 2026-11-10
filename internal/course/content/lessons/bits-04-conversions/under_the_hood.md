**What `bit(31)` was doing.** `1 << 31` is computed as an `int`, where
it's the smallest negative number. Returning it as a `std::uint64_t`
then sign-extends it, as in the last lesson, and you get
`0xffffffff80000000`: 33 bits set instead of one. Shifting by 32 or
more is the undefined behaviour UBSan stopped.

**Turn the warnings on.** `-Wall -Wextra` only warned about one of the
six bugs. Clang has more: `-Wconversion` reports every implicit
narrowing, like the `std::uint8_t sum = a + b` line, and
`-Wsign-conversion` every silent change of signedness. They're noisy
on old code, and worth having from the start on new code.

**Braces refuse to narrow.** `std::uint8_t s{a + b};` is a compile
error, because brace initialisation doesn't allow a conversion that
can lose information. `std::uint64_t{mib}` in the hints is the same
syntax doing a conversion that's always safe.

**Signed sizes.** The standard library's sizes are unsigned for
historical reasons, and many people consider it a mistake. C++20 added
`std::ssize(v)`, which returns the size as a signed number, so
`for (auto i = std::ssize(v) - 1; i >= 0; --i)` works the way it
reads. Ranges help too: `for (int x : v | std::views::reverse)`
has no index to get wrong.

**`char`'s sign depends on the platform.** It's signed on x86-64 and
on Apple's ARM, and unsigned on ARM Linux. Code that puts a `char`
into `std::isalpha` or uses it as a table index works on one and
breaks on the other. `unsigned char` and `std::uint8_t` mean the same
thing everywhere.

**In assembly, the conversions are instructions.** Widening a signed
value is `movsx` on x86-64 (`sxtw` on ARM), and widening an unsigned
one is `movzx` (`uxtw`). When you read compiled code in chapter 4.2,
those two tell you which types the source used.
