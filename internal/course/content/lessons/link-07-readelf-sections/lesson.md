---
title: "mini readelf: sections and symbols"
base: link-06-readelf-header
minutes: 50
files: [sections.cpp]
program: readelf
run:
  args: ["x86-64.o"]
deeper:
  - title: "elf(5): section headers, string tables and symbol tables"
    url: https://man7.org/linux/man-pages/man5/elf.5.html
  - title: "CS:APP 7.4-7.5 again: the sections and symbols of a .o"
    url: https://csapp.cs.cmu.edu/
---
Now the linker's view: the **section header table**, and the
**symbol table** that lesson 5.3.2's `nm` was printing. The header
parsing from last lesson is in `elf.cpp`, finished.

Section header `i` is `e_shentsize` (64) bytes at `e_shoff + i *
e_shentsize`:

| offset | size | field |
|---|---|---|
| 0 | 4 | `sh_name`: where its name starts in the section-name string table |
| 4 | 4 | `sh_type` |
| 8 | 8 | `sh_flags` |
| 16 | 8 | `sh_addr` |
| 24 | 8 | `sh_offset` |
| 32 | 8 | `sh_size` |
| 40 | 4 | `sh_link` |
| 44 | 4 | `sh_info` |
| 48 | 8 | `sh_addralign` |
| 56 | 8 | `sh_entsize` |

Names aren't stored in the headers. A **string table** is a section
of NUL-terminated strings back to back, and a name is an offset into
it: `"\0.text\0.data\0"` has `.text` at 1 and `.data` at 7. The
section names are in section number `e_shstrndx`. That means you
need all the headers before you can name any of them: read them all,
then look the names up.

The **symbol table** is the section of type `SYMTAB` (2). Its
`sh_link` is the number of *its* string table, for the symbol names.
It's an array of `sh_entsize` (24) byte entries:

| offset | size | field |
|---|---|---|
| 0 | 4 | `st_name`: offset in the symbol string table |
| 4 | 1 | `st_info`: binding in the high 4 bits, type in the low 4 |
| 5 | 1 | `st_other`: visibility (skip it) |
| 6 | 2 | `st_shndx`: the section it's defined in, 0 if undefined |
| 8 | 8 | `st_value` |
| 16 | 8 | `st_size` |

Entry 0 is always all zeros. `st_shndx` has special values: `0xfff1`
(`ABS`, an absolute value like the source file name) and `0xfff2`
(`COM`, a "common" uninitialised variable from old C code).

The names, matching `readelf`:

- section types: `NULL` (0), `PROGBITS` (1), `SYMTAB` (2), `STRTAB` (3),
  `RELA` (4), `HASH` (5), `DYNAMIC` (6), `NOTE` (7), `NOBITS` (8),
  `REL` (9), `DYNSYM` (11), `INIT_ARRAY` (14), `FINI_ARRAY` (15),
  `GROUP` (17), `RELR` (19), `GNU_HASH` (`0x6ffffff6`), `VERNEED`
  (`0x6ffffffe`), `VERSYM` (`0x6fffffff`), anything else in hex
- section flags, in this order: `W` (1, writable), `A` (2, takes memory
  when loaded), `X` (4, executable), `M` (`0x10`, mergeable), `S`
  (`0x20`, strings), `I` (`0x40`, `sh_info` is a section number)
- symbol types: `NOTYPE` (0), `OBJECT` (1), `FUNC` (2), `SECTION` (3),
  `FILE` (4), `TLS` (6); bindings: `LOCAL` (0), `GLOBAL` (1), `WEAK` (2)
- section indexes: `UND`, `ABS`, `COM`, or the number

Read the sample object's table and compare it with `sample.c` at the
top of `samples.hpp`: `hidden` is a `LOCAL` in `.bss`, `add` a
`GLOBAL FUNC` in `.text`, and `puts` is `UND`, waiting for the linker.

## Assignment

- [ ] `string_at` reads a NUL-terminated string from a section, and throws `std::out_of_range` outside it
- [ ] `parse_sections` reads every header, then names them from section `e_shstrndx`
- [ ] `parse_symbols` reads the `SYMTAB` section, named from the string table at its `sh_link`
- [ ] The five name functions match `readelf`
- [ ] Run it on the samples, and on Linux on `/bin/ls`. Where did `/bin/ls`'s symbols go?
