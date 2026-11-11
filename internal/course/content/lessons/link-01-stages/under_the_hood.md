**What `clang++` actually runs.** `clang++ -v main.cpp` prints every
step. Clang does preprocessing, compiling and assembling inside one
process (`clang -cc1`) and skips writing the in-between files, then
runs the linker: `ld` on Linux (often `ld.lld` or GNU `ld` through
`collect2`), `ld` from Apple on the Mac. The link line it prints is
long, because it adds the C runtime start-up files (`crt1.o`,
`crti.o`), the C++ standard library and libc.

**Why one file at a time?** It's how C was designed in 1972, when
memory was too small to hold a whole program, and it's why build
systems can recompile only the files that changed, and compile many
at once. The price is that the compiler can't see across files
(lesson 5.2.1's LTO pays for that), and that every header is parsed
again in every file that includes it. A big C++ project spends most
of its build time re-parsing the same standard headers.

**Precompiled headers and modules.** Precompiled headers save the
compiler's state after parsing common headers. C++20 **modules**
(`import std;`) replace textual inclusion: a module is compiled once
into a binary interface that importers load. They also stop macros
from leaking out of a header into everything that includes it.

**Include guards.** `#pragma once` (or the older `#ifndef FOO_H`
`#define FOO_H` … `#endif`) stops a header being pasted in twice in
one translation unit. It does nothing across translation units: each
`.cpp` file still gets its own copy. That's why a function *defined*
in a header can end up defined in several object files, which is the
next lesson.

**`assert` is a macro.** It uses `#cond` for the message and
`__FILE__`/`__LINE__` for the place, and when `NDEBUG` is defined it
expands to nothing at all. That's why the condition must never have
side effects: in a release build, it isn't evaluated. C++20's
`std::source_location::current()` gives the file and line without a
macro.
