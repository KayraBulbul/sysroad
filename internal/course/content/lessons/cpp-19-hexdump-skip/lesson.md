---
title: "hexdump: -s, -n and squeezing repeats"
minutes: 20
files: [hexdump.cpp]
run:
  args: ["-C", "zeros.bin"]
  files:
    zeros.bin: "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0end\n"
---
Real files are full of repetition, like zero padding and blank sectors,
and hexdump folds it away:

```text
$ head -c 64 /dev/zero | hexdump -C
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
*
00000040
```

A `*` means "the lines from here on are the same as the one above". The
next different line shows up with its real offset, so you can tell how much
was folded.

**The spec**

- **Squeezing:** a full 16-byte line that's identical to the previous full
  line prints a single `*`, once for the whole run. Keep comparing against
  the previous line whether it was printed or squeezed. `-v` turns
  squeezing off.
- **`-s N`** skips the first N bytes of the input. Offsets still count
  them, so the first line starts at offset N. If the input is shorter than
  N, nothing is dumped, but the final offset still prints the length.
- **`-n N`** dumps at most N bytes (after the skip). `-n 0` prints nothing
  at all. That's already handled in `run_hexdump`.
- The final offset line appears if any byte was read *or skipped*.

Numbers are decimal or `0x` hex, and come as the next argument (`-s 16`)
or stuck on (`-s16`). Parsing is done for you (`parse_number` uses
`std::from_chars`, which is worth reading). Your work is in `Dumper`, which
now gets the options in its constructor.

## Assignment

- [ ] Skip the first `-s` bytes in `feed`, across as many chunks as it takes
- [ ] Stop after `-n` bytes
- [ ] Squeeze repeated full lines into `*` in `line`, unless `-v`
