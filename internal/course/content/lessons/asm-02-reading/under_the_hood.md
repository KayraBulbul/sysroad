**`.cfi` directives are for unwinding.** CFI is "call frame
information": a table that says, for every instruction, how to find
the caller's frame. The debugger uses it for `bt`, and exceptions use
it to walk up the stack running destructors. It never runs as code.
`-fno-asynchronous-unwind-tables` leaves it out of the listing.

**Flags that make listings nicer.** `-fno-asynchronous-unwind-tables`
drops the `.cfi` lines, `-fverbose-asm` adds comments naming the
variables, and on x86-64 `-masm=intel` switches to Intel syntax. For a
built program, `objdump -d --no-show-raw-insn -C` hides the machine
code bytes and demangles names, and `-M intel` switches syntax there.

**What mangled names say.** `_ZN3geo4areaEii` reads as: `_Z` starts
every mangled name, `N…E` wraps a nested name, `3geo` and `4area` are
each a length and that many letters, and `ii` is two `int` parameters.
The return type isn't part of it, which is why you can't overload on
the return type alone.

**`@PLT`.** `puts` lives in the C library, a shared library loaded at
run time, so its address isn't known when your code is compiled. The
call goes to a small stub in the *procedure linkage table*, which
looks the real address up on first use. Chapter 5.3 takes this apart.

**A jump instead of a call.** `greet` ends with `jmp puts@PLT` (or
`b _puts`), not a call. Calling `puts` was the last thing `greet` did,
so instead of calling and then returning, it jumps there and lets
`puts` return straight to `greet`'s caller. That's a **tail call**,
and you'll meet it again in lesson 4.2.6.

**Sections.** `.text` holds code, `.data` holds globals that start
with a value, `.rodata` holds constants and string literals, and
`.bss` holds globals that start as zero. On the Mac they're
`__TEXT,__text`, `__DATA,__data` and `__TEXT,__cstring`. Writing to a
string literal crashes because `.rodata` is mapped read-only.

**The assembler is a separate step.** `clang++ -c file.s` turns a
listing into an object file, and `clang++ file.s -o prog` goes all the
way to a program. A compiler is a text-to-text translator with an
assembler and a linker run after it.
