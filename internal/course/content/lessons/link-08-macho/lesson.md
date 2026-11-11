---
title: Mach-O on the Mac
minutes: 50
files: [macho.cpp]
program: machotool
run: {}
deeper:
  - title: "Apple's mach-o/loader.h (every structure, in the SDK)"
    url: https://github.com/apple-oss-distributions/xnu/blob/main/EXTERNAL_HEADERS/mach-o/loader.h
  - title: "Mach-O Programming Topics (Apple, archived)"
    url: https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/MachOTopics/0-Introduction/introduction.html
  - title: "Parsing Mach-O files (Alex Denisov)"
    url: https://lowlevelbits.org/parsing-mach-o-files/
---
Your Mac doesn't use ELF. Its programs, libraries and object files
are **Mach-O**, from the Mach kernel that macOS grew out of. Same
problems, different shapes. Where ELF has two tables, Mach-O has one
list of **load commands** straight after a 32-byte header, and each
command says what it is and how long it is:

```text
mach_header_64 (32 bytes)
  0  magic        0xfeedfacf
  4  cputype      0x0100000c arm64, 0x01000007 x86-64
  8  cpusubtype
 12  filetype     1 object, 2 executable, 6 dylib, 8 bundle
 16  ncmds        how many load commands
 20  sizeofcmds   their total size
 24  flags
 28  reserved
load command 0:   cmd (4), cmdsize (4), then cmdsize - 8 bytes depending on cmd
load command 1:   …
```

Commands follow each other with no gaps, each `cmdsize` bytes long.
Code that doesn't know a command skips it by its size, which is how
old tools cope with new commands. The ones you'll read:

- **`LC_SEGMENT_64`** (`0x19`): a segment, like an ELF `LOAD`, with its
  sections inside it. A segment has `segname` (16 bytes at offset 8),
  `vmaddr`, `vmsize`, `fileoff` and `filesize` (8 bytes each from
  offset 24), then `maxprot`, `initprot`, `nsects` (at 64) and
  `flags`: 72 bytes. Then `nsects` sections of 80 bytes each:
  `sectname` (16), `segname` (16), `addr` (8 at 32), `size` (8 at 40),
  `offset` (4 at 48), and fields you can skip. Names are padded with
  NULs, and a name that fills all 16 bytes has no NUL at all.
- **`LC_LOAD_DYLIB`** (`0xc`) and **`LC_LOAD_WEAK_DYLIB`**
  (`0x80000018`): a library the program needs. At offset 8 is the
  offset of its name, *counted from the start of the command*; the
  name runs to a NUL inside the command.
- **`LC_SYMTAB`** (`0x2`): `symoff`, `nsyms`, `stroff` and `strsize`, 4
  bytes each from offset 8. The symbols are 16-byte `nlist_64`
  entries at `symoff`: `n_strx` (4, an offset in the string table at
  `stroff`), `n_type` (1), `n_sect` (1), `n_desc` (2), `n_value` (8).

A Mach-O object file has a single segment with no name, holding every
section. An executable has the real ones: `__PAGEZERO` (4 GB of
nothing at address 0, so a null pointer always faults), `__TEXT`
(code and constants), `__DATA_CONST`, `__DATA` and `__LINKEDIT` (the
symbol table, signature and loader data). Symbol names start with an
underscore, which C compilers on Apple platforms add to every name:
`main` is `_main`.

`command_name` follows `otool`: `LC_SYMTAB` (`0x2`), `LC_DYSYMTAB`
(`0xb`), `LC_LOAD_DYLIB` (`0xc`), `LC_LOAD_DYLINKER` (`0xe`),
`LC_SEGMENT_64` (`0x19`), `LC_UUID` (`0x1b`), `LC_CODE_SIGNATURE`
(`0x1d`), `LC_FUNCTION_STARTS` (`0x26`), `LC_DATA_IN_CODE` (`0x29`),
`LC_SOURCE_VERSION` (`0x2a`), `LC_LINKER_OPTIMIZATION_HINT` (`0x2e`),
`LC_BUILD_VERSION` (`0x32`), `LC_LOAD_WEAK_DYLIB` (`0x80000018`),
`LC_DYLD_INFO_ONLY` (`0x80000022`), `LC_MAIN` (`0x80000028`),
`LC_DYLD_EXPORTS_TRIE` (`0x80000033`) and `LC_DYLD_CHAINED_FIXUPS`
(`0x80000034`). Anything else in hex.

On the Mac, run it on `/bin/ls` and compare with `otool -l /bin/ls`
and `otool -L /bin/ls`. `/bin/ls` is a **universal** binary, holding
an x86-64 and an arm64 program; `main.cpp` picks out the arm64 one
before your code sees it.

## Assignment

- [ ] `parse_mach_header` checks the magic (with a clear error for universal binaries) and reads the header
- [ ] `load_commands` walks `ncmds` commands by their sizes, and throws for a size under 8 or past the end
- [ ] `segments` reads each `LC_SEGMENT_64` and its sections
- [ ] `dylibs` and `symbols` read the libraries and the symbol table
- [ ] `command_name` matches `otool`
