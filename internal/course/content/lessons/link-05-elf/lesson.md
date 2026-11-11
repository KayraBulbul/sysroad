---
title: The ELF format
minutes: 35
files: [ident.cpp]
program: ident
run:
  args: ["/bin/ls", "hello.sh", "notes.txt"]
  files:
    hello.sh: "#!/bin/sh\necho hi\n"
    notes.txt: "just some text\n"
deeper:
  - title: "CS:APP 7.4-7.5: relocatable object files and symbols"
    url: https://csapp.cs.cmu.edu/
  - title: "elf(5) man page: every structure, field by field"
    url: https://man7.org/linux/man-pages/man5/elf.5.html
  - title: "In-depth: ELF, the Extensible & Linkable Format (stacksmashing)"
    url: https://www.youtube.com/watch?v=nC1U1LJQL8o
---
Object files, executables, shared libraries and core dumps on Linux
are all one format: **ELF**, the Executable and Linkable Format. The
next two lessons build `readelf`, so first, a map of the file:

```text
+------------------------+  offset 0
| ELF header (64 bytes)  |  what this is, and where the two tables are
+------------------------+
| program header table   |  segments: what the loader maps, and where
+------------------------+
| .text                  |  machine code
| .rodata                |  constants and string literals
| .data                  |  initialised globals
| .symtab, .strtab       |  symbols and their names
| .rela.text             |  relocations: addresses the linker must fill in
| …                      |
+------------------------+
| section header table   |  sections: what the linker works with
+------------------------+
```

Two views of the same bytes. The **linker** works with **sections**,
named pieces like `.text` and `.data`, and combines the `.text` of every
object file into one `.text`. The **loader** doesn't care about names.
It reads **segments** from the program header table: "map these bytes
at this address, read-only and executable". A `.o` file has sections
and no segments, because nothing loads it directly. An executable has
both, and only needs the segments to run. `.bss` (zero-initialised
globals) takes no space in the file at all: a segment just says to map
more memory than the file holds.

Every ELF file starts with 16 identification bytes:

```text
7f 45 4c 46   "\x7fELF"
02            class: 1 = 32-bit, 2 = 64-bit
01            data: 1 = little-endian, 2 = big-endian
01            version, always 1
00            OS/ABI: 0 = System V (Linux uses it too)
00 …          padding to 16 bytes
```

The class and byte order come first because they decide how to read
everything after them. Then come 2-, 4- and 8-byte numbers. On
x86-64 and arm64 they're **little-endian**: least significant byte
first, so the bytes `01 02` are the number `0x0201`. Build them from
bytes with shifts, as you did in phase 4, and never by casting a
`char*` to a `std::uint32_t*`: the bytes may not be aligned, and the
compiler is allowed to assume they are.

Other formats have magic numbers too. A Mac's Mach-O starts with
`0xfeedfacf` (stored little-endian, so the bytes are `cf fa ed fe`).
A **universal** ("fat") binary, holding code for several CPUs, starts
with the bytes `ca fe ba be`, and so does a Java class file. `file(1)`
tells them apart by the next four bytes, read big-endian: a fat binary
has a small count of CPU types there, a class file its version
number, 45 or more.

## Assignment

- [ ] `read_u16`, `read_u32` and `read_u64` build little-endian numbers from bytes, and throw `std::out_of_range` past the end
- [ ] `identify` recognises ELF, with its class and byte order
- [ ] It recognises the four Mach-O magics, universal binaries, Java classes, archives, scripts and `MZ`
- [ ] Run it, then `readelf -h /bin/ls` (Linux) or `otool -h /bin/ls` (Mac)
