**`isspace` is a table lookup.** In glibc, `std::isspace(c)` reads a
bitmask from a per-locale table, roughly `table[c] & _ISspace`. That's why
a negative `c` is dangerous: it indexes *before* the table. glibc pads the
table so that indexes from -128 to -1 happen to work, but the standard
still calls it undefined, and other C libraries don't pad.

**GNU wc understands characters, not just bytes.** In a UTF-8 locale, GNU
wc decodes multibyte characters (`mbrtowc`) and asks `iswspace`, so a few
Unicode spaces count as separators. Your version treats every byte ≥ 0x80
as part of a word. For ASCII text (and `LC_ALL=C wc`) the two always agree.

**Counting lines can be faster than counting words.** `wc -l` only looks for
`'\n'`, so it can use `memchr` or, in recent coreutils, AVX2 instructions
that check 32 bytes at once. Counting words needs the whitespace/word state
at every byte, so it's a plain loop like yours.

**Why one pass matters.** Your counter keeps a single `bool` of state. In
the next lesson, text arrives in 4 KiB chunks and a word can be split
across two of them. A counter whose whole state is one `bool` can carry
that across the boundary. A counter that needs the whole text can't.
