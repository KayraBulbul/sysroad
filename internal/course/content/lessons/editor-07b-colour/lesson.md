---
title: "Drawing in colour: numbers, strings, the match"
kind: capstone
project: kite
program: kite
minutes: 45
run:
  args: [demo/sample.cxx]
  interactive: true
deeper:
  - title: "Build Your Own Text Editor: syntax highlighting"
    url: https://viewsourcecode.org/snaptoken/kilo/07.syntaxHighlighting.html
  - title: "ANSI escape code: colours (Wikipedia)"
    url: https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
---
kite starts colouring C and C++ files: this lesson the machinery and
two kinds of text, numbers and strings; next lesson the rest of C++.

**One colour per character.** Each row gets a third array, next to
`chars` and `render`: one entry per character of `render`, saying what
it is. An `enum class` makes a good entry, `Normal`, `Number` and
`String` for now, with more next lesson. Working it out is a walk along
`render`, left to right, keeping a little state: whether you're inside
a string (and which quote opened it), and whether the last character
was a **separator**, the end of a word. At each position, the first
rule that fits wins:

```text
inside a string     String; a backslash takes the next character too; the opening quote closes it
" or '              starts a string
a digit             Number, if it starts a word (or continues a number)
. or a letter       Number too, right after a number: 3.14f, 0xff, 10u
anything else       Normal
```

The separators are whitespace, the NUL byte, and
`,.()+-/*=~%<>[]{};:!&|^?`. The start of a line counts as one, and so
does the end of a string. So `x2` is a name, not a number, and `a[2]`
has a number in it. Only files whose names end in `.c`, `.h`, `.cc`,
`.cpp`, `.cxx`, `.hh`, `.hpp` or `.hxx` are coloured. Every other file
is all `Normal`, and so is a C++ file that's saved under a new name
that isn't one.

The colours have to stay right as you type, so every change to a row's
`chars` now remakes its colours as well as its `render`. One helper
that does both, called everywhere `render` was remade, keeps that in
one place.

**Drawing it.** `draw_screen` now writes colour codes as it goes, the
16-colour palette's so kite follows your terminal's theme:

```text
\x1b[31m   red, for numbers           \x1b[35m   magenta, for strings
\x1b[39m   the terminal's own colour  \x1b[m     everything back to normal
```

Write a code only when the colour changes from the last character, not
before every character, and end each row with everything back to
normal, so the colour doesn't leak into the next row or the bars.

**Control characters.** A file can hold bytes like ESC or `\r`, and
written to the terminal as they are, they act: an ESC in a file could
move the cursor or change the colours. kite shows them safely instead,
inverted, as the letter Ctrl would make them: byte 1 as an inverted
`A`, ESC (27) as `[`, and DEL (127) as `?`. That goes for every file.

**The match.** While a search is on, the characters of the current
match are inverted too, so you can see what it found. When the search
ends, by Enter or Escape, they go back to their colours.

## Contract

Nothing new. The tests read the colours off the screen, from the codes
your frames contain. `\x1b[m` and `\x1b[0m`, or `\x1b[39m`, all come
out the same, as long as each character ends up the right colour.

## Assignment

- [ ] In a C or C++ file, numbers are red and strings magenta, by the rules above; everything else is the terminal's own colour
- [ ] Only files with those eight endings get colours, and saving under a new name decides again
- [ ] As you edit, the colours change with the text
- [ ] Control characters show inverted, as `@` plus their value (`?` for DEL), in every file
- [ ] While searching, the match is inverted; after Enter or Escape it isn't
