**Try it on real files.** `readelf -h` prints the header, `-S` the
sections, `-l` the segments and which sections each one holds, `-s`
the symbols and `-r` the relocations. `objdump -d` disassembles
`.text`. On the Mac, `otool -h`, `otool -l` and `nm` do the same for
Mach-O, and Homebrew's `binutils` adds `readelf` for reading Linux
files.

**Relocations.** When `add.o` is compiled, the compiler doesn't know
where `hidden` will end up, so the instruction that reads it has a
placeholder, and `.rela.text` holds an entry saying "at offset 5 in
`.text`, put the address of `hidden`, PC-relative, minus 4". The
linker lays out all the sections, then works through every relocation
and patches the bytes. That's the second half of linking, after
symbol resolution. `objdump -dr` shows each relocation under the
instruction it patches.

**Why there are two tables.** Sections are fine-grained and named,
which the linker needs. Segments are coarse, page-aligned and few,
which the loader wants: mapping one segment is one `mmap`. A typical
executable has 30 sections and 4 loadable segments: read-only data,
code, read-only after relocation (`RELRO`), and writable data. A
stripped executable can have its section headers removed entirely and
still runs.

**Core dumps are ELF too.** A core file is an ELF of type `CORE`: one
segment per memory mapping of the crashed process, plus notes holding
the registers of each thread. That's how `gdb prog core` (or
`coredumpctl gdb`) can show you the stack of a program that's no
longer running.

**Other formats.** Windows uses PE (Portable Executable), which still
starts with the `MZ` of 1980s MS-DOS programs and a stub that prints
"This program cannot be run in DOS mode". Apple uses Mach-O (lesson
5.3.8). WebAssembly modules start with `\0asm`. All of them solve the
same problems: code, data, symbols, relocations and how to load it.
