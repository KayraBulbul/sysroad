**Real layouts are tighter.** Yours is 40 bytes. libstdc++'s
`std::string` is 32: a pointer (to the heap, or to its own inline
buffer), a size, and a 16-byte union that's either the inline text or
the capacity. libc++ packs it into 24 bytes. A flag bit says "short",
and a short string's size, text and `'\0'` all fit in those 24 bytes,
which gives 22 inline chars. Folly's `fbstring` stores 23 by using the
*last* byte as both "remaining capacity" and the terminator.

**libstdc++'s self-pointer.** Pointing at its own inline buffer means
`data()` needs no branch. The cost is that a libstdc++ string can't be
moved with `memcpy`: the copy's pointer would still point into the
original. Your `heap_ ? heap_ : buf_` branch avoids that, and the
branch is nearly free.

**Copy-on-write is gone.** Before C++11, libstdc++ shared the buffer
between copies until one of them was written to. C++11 effectively
banned it: `operator[]` could no longer cause a copy, and reference
counts are slow with threads. SSO replaced it.

**Why keep the `'\0'`?** So `c_str()` is free. C APIs (`open`, `printf`,
`getenv`) want terminated strings, and a `std::string` always keeps
one past the end, even though it also stores its size and can hold
`'\0'` in the middle.

**Growth factor.** libstdc++ and libc++ double. Some vectors (Folly's,
MSVC's) grow by 1.5×, which lets freed blocks be reused for later
growth. Phase 7's allocator shows why that matters.
