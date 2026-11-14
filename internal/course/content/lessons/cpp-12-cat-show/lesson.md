---
title: "-E, -T, -v, -A: showing the invisible"
base: cpp-11-cat-squeeze
minutes: 15
files: [cat.cpp]
run:
  args: ["-A", "dos.txt"]
  files:
    dos.txt: "name\tvalue\r\nbell\x07\r\n"
deeper:
  - title: Caret notation (Wikipedia)
    url: https://en.wikipedia.org/wiki/Caret_notation
---
Some bytes don't show up on screen: trailing spaces, tabs, Windows line
endings, stray control characters. `cat -A` makes them visible:

```text
$ printf 'name\tvalue\r\n' | cat -A
name^Ivalue^M$
```

- `-E` writes `$` before each newline, so trailing spaces show. A `\r`
  directly before a newline (a Windows line ending) shows as `^M` too,
  even without `-v`.
- `-T` writes each tab as `^I`.
- `-v` writes the other non-printing bytes in **caret notation**:
  - bytes 0–31 become `^` followed by the byte plus 64: 1 is `^A`, 13
    (`\r`) is `^M`, and 0 is `^@`. Tab and newline are left alone.
  - 127 (DEL) is `^?`.
  - 128–255 get `M-` ("meta"), then the byte minus 128 written the same
    way, so 255 is `M-^?` and 0xC3 is `M-C`.
- `-A` is `-vET`, `-e` is `-vE`, and `-t` is `-vT`.

Work with the byte as an `unsigned char`, so 0xC3 is 195 and not -61:
`show_byte(text, static_cast<unsigned char>(ch))`.

That `\r` rule needs one byte of lookahead: when you see a `\r` you can't
know yet whether a `\n` follows. It might even be in the next chunk, or the
next file. So hold it back in `st.pending_cr`, and decide when the next byte
arrives: `^M` if it's a newline, a plain `\r` otherwise. `run_cat` already
writes out a `\r` that's still held at the very end.

The flags combine with numbering and squeezing, which run first, at the
start of the line. Then the byte itself is written, its way.

## Assignment

- [ ] Implement `show_byte` for `-v`
- [ ] Write each byte the right way: `$` before newlines, `^I` for tabs, `show_byte` for the rest
- [ ] With `-E`, hold back a `\r` and show it as `^M` when a newline follows
