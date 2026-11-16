---
title: Reading keys
kind: capstone
project: kite
program: kite
minutes: 40
contract:
  functions:
    - name: decode_key
      sig: "std::pair<int, std::size_t>(std::string_view)"
run:
  interactive: true
deeper:
  - title: "ANSI escape code: CSI sequences (Wikipedia)"
    url: https://en.wikipedia.org/wiki/ANSI_escape_code#CSI_(Control_Sequence_Introducer)_sequences
  - title: "xterm control sequences: PC-style function keys"
    url: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-PC-Style-Function-Keys
---
Some keys aren't one byte. The arrows, Home, End and friends arrive as
**escape sequences**: ESC (`\x1b`), then `[`, then a letter, or a
number and `~`. Run last lesson's kite and press them: Up is
`27 91 65`, which is `\x1b[A`, and Delete is `\x1b[3~`.

```text
ESC [ A, B, C, D           Up, Down, Right, Left
ESC [ H, ESC O H           Home    (also ESC [ 1 ~ and ESC [ 7 ~)
ESC [ F, ESC O F           End     (also ESC [ 4 ~ and ESC [ 8 ~)
ESC [ 3 ~                  Delete
ESC [ 5 ~, ESC [ 6 ~       Page Up, Page Down
```

They all have one shape: ESC `[`, then **parameter** bytes (digits and
`;`, `0x30` to `0x3f`), then one **final** byte from `@` to `~`
(`0x40` to `0x7e`). That shape matters for the keys kite doesn't use.
F5 is `\x1b[15~` and Ctrl-Right is `\x1b[1;5C`, and if kite read them a
byte at a time, typing F5 would insert `[15~` into your file. So any
other whole sequence is one more key, **Unknown**, that does nothing.
The same goes for `ESC O` and one more byte, like F1's `\x1bOP`.

`decode_key` turns bytes into keys. It takes the bytes read so far and
returns the first key in them, and how many bytes that key used. An
ordinary key is its byte value, 0 to 255. Read it as an `unsigned
char`, or bytes above 127 (UTF-8 text) come out negative. The other
keys need values above 255, so they can't be mistaken for a byte. An
enum gives them names:

```cpp
enum Key : int { kArrowLeft = 1000, kArrowRight, kArrowUp, /* ... */ };
```

Then there's the **Escape** key, which sends a lone ESC, the same byte
every sequence starts with. Only time tells them apart: a sequence's
bytes arrive together, in one `read` or within a moment, and an Escape
press is followed by nothing. So a key reader keeps the bytes it has
read but not used yet, and when what it has decodes as Escape, it
reads again, which waits at most a tenth of a second (`VTIME`), and
decodes again. It keeps doing so while what it has still decodes as
Escape and could be the start of a sequence (a lone ESC, or an
unfinished `ESC [` or `ESC O`): over a slow link, `ESC`, `[` and `A`
can each arrive on their own. When nothing more comes, it's Escape.
ESC on its own, ESC followed by an unfinished sequence, or ESC followed
by anything else (Alt-x sends `\x1bx`) is Escape, using 1 byte.

kite now prints the **name** of each key, one per line. The tests type
at it and read the names, so spell them like this:

```text
Up  Down  Left  Right  Home  End  PageUp  PageDown  Delete  Unknown
Esc  Enter  Tab  Backspace  Space          the keys with those bytes: 27, 13, 9, 127, 32
Ctrl-A ... Ctrl-Z                          the other bytes from 1 to 26
!  a  ~                                    other printable bytes, as themselves
byte 200                                   anything else
```

## Contract

```cpp
// The first key in bytes, and how many bytes it used. bytes holds at
// least one byte.
std::pair<int, std::size_t> decode_key(std::string_view bytes);
```

Your key values are your own. The tests check that a key is a byte
where it should be and above 255 where it should be, and compare keys
with each other: Home is the same key whichever of its four spellings
arrives.

## Assignment

- [ ] `decode_key` returns an ordinary byte as itself (0 to 255), using 1 byte
- [ ] The sequences in the table are their keys, with values above 255, and use all their bytes
- [ ] Any other whole `ESC [` sequence is Unknown, using all its bytes, and so is `ESC O` and any other byte, using 3. Unknown is one more key, above 255, that isn't any of the others
- [ ] ESC alone, an unfinished sequence, or ESC then anything else is Escape (27), using 1 byte
- [ ] kite reads keys, giving a sequence a moment to arrive after its ESC, and prints each key's name on a line of its own
