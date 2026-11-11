---
title: "mini readelf: the header"
minutes: 45
files: [elf.cpp]
program: readelf
run:
  args: ["x86-64-exe"]
deeper:
  - title: "elf(5): Elf64_Ehdr and Elf64_Phdr"
    url: https://man7.org/linux/man-pages/man5/elf.5.html
  - title: "System V ABI: the ELF specification (gABI)"
    url: https://www.sco.com/developers/gabi/latest/contents.html
  - title: "How programs get run: ELF binaries (LWN)"
    url: https://lwn.net/Articles/631631/
---
Time to build `readelf`. This lesson reads the **ELF header** and the
**program header table** (`readelf -h -l`); the next adds sections and
symbols. It reads 64-bit little-endian files, which covers x86-64 and
arm64 Linux. On the Mac it reads the built-in samples, which `main.cpp`
offers by name: `x86-64-exe` (the default), `x86-64.o` and `arm64.o`.
On Linux, try it on `/bin/ls` too.

The header is 64 bytes at offset 0. After the 16 identification bytes
from lesson 5.3.5:

| offset | size | field | |
|---|---|---|---|
| 16 | 2 | `e_type` | 1 relocatable (`.o`), 2 executable, 3 shared object, 4 core |
| 18 | 2 | `e_machine` | 62 x86-64, 183 AArch64 |
| 20 | 4 | `e_version` | 1 |
| 24 | 8 | `e_entry` | the address `_start` is at, or 0 |
| 32 | 8 | `e_phoff` | file offset of the program header table |
| 40 | 8 | `e_shoff` | file offset of the section header table |
| 48 | 4 | `e_flags` | CPU-specific |
| 52 | 2 | `e_ehsize` | 64 |
| 54 | 2 | `e_phentsize` | size of one program header: 56 |
| 56 | 2 | `e_phnum` | how many |
| 58 | 2 | `e_shentsize` | size of one section header: 64 |
| 60 | 2 | `e_shnum` | how many |
| 62 | 2 | `e_shstrndx` | which section holds the section names |

Program header `i` is at `e_phoff + i * e_phentsize`. Use `e_phentsize`
rather than 56, the way real tools do, so a future version with bigger
entries still reads. Each one is a **segment**:

| offset | size | field | |
|---|---|---|---|
| 0 | 4 | `p_type` | 1 `LOAD`, 2 `DYNAMIC`, 3 `INTERP`, 4 `NOTE`, 6 `PHDR`, 7 `TLS` |
| 4 | 4 | `p_flags` | 4 read, 2 write, 1 execute |
| 8 | 8 | `p_offset` | where its bytes start in the file |
| 16 | 8 | `p_vaddr` | where they go in memory |
| 24 | 8 | `p_paddr` | physical address: unused, skip it |
| 32 | 8 | `p_filesz` | how many bytes come from the file |
| 40 | 8 | `p_memsz` | how much memory: more than `p_filesz` for `.bss` |
| 48 | 8 | `p_align` | |

Types `0x6474e550` to `0x6474e554` are GNU extensions: `GNU_EH_FRAME`,
`GNU_STACK`, `GNU_RELRO`, `GNU_PROPERTY` and `GNU_SFRAME`. Print
any other type as hex, `0x70000001`.

For the names, match `readelf`: `type_name` gives `NONE (None)`,
`REL (Relocatable file)`, `EXEC (Executable file)`,
`DYN (Shared object file)` and `CORE (Core file)`, and
`machine_name` gives `Intel 80386` (3), `ARM` (40),
`Advanced Micro Devices X86-64` (62), `AArch64` (183) and `RISC-V`
(243). Anything else is `<unknown>: 0x…` in hex, which
`std::format("<unknown>: {:#x}", value)` makes. `segment_flags` is
three characters, `R`, `W` and `E` or a space each: `"R E"`.

`bytes.hpp` has the course's `read_u16`, `read_u32` and `read_u64`.
They throw `std::out_of_range` past the end, so a truncated file
fails cleanly. Throw `std::runtime_error` yourself if the magic is
wrong, or the file isn't 64-bit little-endian.

## Assignment

- [ ] `parse_header` checks the magic, class and byte order, and reads every field in `ElfHeader`
- [ ] `parse_segments` reads `phnum` program headers, `phentsize` bytes apart
- [ ] The four name functions match `readelf`
- [ ] On Linux, compare `readelf /bin/ls` with `readelf -h -l -W /bin/ls`
