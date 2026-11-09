**A span is two words.** `sizeof(std::span<int>)` is 16: a pointer and
a `size_t`. Pass it by value, like `std::string_view`. There's also a
fixed-size form, `std::span<int, 4>`, which is just the pointer, since
the length is part of the type.

**Why pointer overflow is UB.** The standard only defines pointers
within one object (plus one past the end). That lets compilers treat
pointers as plain numbers that never wrap. GCC once deleted the check
`if (buf + len < buf)` from real code, because with that rule it can
never be true. Several Linux kernel checks broke the same way. Bounds
checks compare lengths, never pointers that might be out of range.

**Hardened mode.** libc++ and libstdc++ can check `span` and `vector`
indexing at runtime (`-D_LIBCPP_HARDENING_MODE=...` /
`-D_GLIBCXX_ASSERTIONS`). Several big projects ship with this on now,
since the check costs very little.

**Parsers are attack surface.** Image, font and archive parsers are
where a lot of real security bugs live. A file says "here come 4
billion bytes" and the code believes it. A reader that refuses to run
off the end, like yours, closes off a whole family of these bugs.
`main.cpp` also reads the player count from the file and still can't be
tricked into reading past it.
