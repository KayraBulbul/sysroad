**Lazy binding.** By default (without `RTLD_NOW` or `-z now`), a PLT
entry starts out pointing at the loader itself. The first call to
`printf` goes to the loader, which looks up the real address, writes
it into the GOT and jumps there. Later calls go straight through. A
program that calls 2 of a library's 2,000 functions resolves 2.
Hardened distributions link with `-z now -z relro` anyway: resolve
everything at start-up and make the GOT read-only, so an attacker
can't overwrite it.

**`LD_PRELOAD`.** `LD_PRELOAD=./libmine.so prog` loads your library
first, and the loader takes each symbol from the first library that
defines it. Define `malloc` in it, and every `malloc` in the program
and its libraries calls yours: that's how memory profilers, `faketime`
and some debugging tools work, with no recompiling. On the Mac,
`DYLD_INSERT_LIBRARIES` does the same, but not for Apple's own
programs, which System Integrity Protection protects.

**How glibc picks a `memchr`.** glibc ships several versions of
`memchr`, `strlen` and `memcpy`: for SSE2, AVX2, AVX-512 and more. The
symbol is an **IFUNC**: instead of an address, it points at a small
resolver function that the loader calls once, at start-up. It checks
the CPU with `cpuid` and returns the best version, and that address
goes in the GOT. Your program gets AVX2 `memchr` on a CPU that has it
without knowing it exists.

**Symbol versions.** `objdump -T /usr/lib/libc.so.6 | grep memcpy`
shows `memcpy@GLIBC_2.14` and `memcpy@GLIBC_2.2.5`: glibc keeps old
behaviour available under old version tags, so programs linked years
ago still get what they were built against. It's also why a program
built on a new Linux can fail on an old one with "version
`GLIBC_2.34' not found".

**The Mac's dyld shared cache.** On macOS 11 and later, the system
libraries aren't individual files on disk any more. They're
pre-linked together into one big cache that every process maps, which
makes start-up faster. `dlopen("/usr/lib/libSystem.B.dylib")` still
works: dyld finds it in the cache.

**Exporting less.** Every non-static function in a shared library is
exported by default, which slows loading and makes it easy to depend
on internals. Building with `-fvisibility=hidden` and marking the
real API with `__attribute__((visibility("default")))` exports only
what you mean to.
