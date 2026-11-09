---
title: Syntax highlighting for C++
kind: capstone
program: kite
minutes: 40
files: [highlight.cpp]
run:
  args: [demo/sample.cpp]
  interactive: true
deeper:
  - title: "Build Your Own Text Editor: syntax highlighting"
    url: https://viewsourcecode.org/snaptoken/kilo/07.syntaxHighlighting.html
  - title: "Tree-sitter"
    url: https://tree-sitter.github.io/tree-sitter/
---
kite colours C and C++ files. Each row gets a third array, `hl`, with
one `Hl` per character of `render`: `Keyword`, `Type`, `String`,
`Number`, `Comment` and so on. The course's `draw_screen` turns them
into colour escape codes, like `\x1b[33m` for yellow. You decide what
each character is:

```text
int n = 42;   // the answer
tttnnnnnddnnncccccccccccccc
```

`highlight_row` walks `render` left to right, keeping a little state:
whether it's inside a string (and which quote opened it), whether the
last character was a **separator** (the end of a word), and whether
it's inside a `/* */` comment. That last one can span lines, so it
comes in as a parameter and goes out as the return value. When a row's
answer changes, the course re-highlights the rows after it.

At each position, the first rule that fits wins:

```text
//                  a comment to the end of the line
inside /* */        MlComment, until */ (both characters included)
/*                  starts one
inside a string     String; a backslash takes the next character too; the opening quote closes it
" or '              starts a string
a digit             Number, if it starts a word (or continues a number)
. or a letter       Number too, right after a number: 3.14f, 0xff, 10u
a keyword or type   as a whole word: at the start of a word, with a separator or the line's end after it
# first on a line   Preproc, and so is the rest of the line, except strings and comments
anything else       Normal
```

Only comment markers outside strings count, and only string quotes
outside comments.

## Assignment

- [ ] `is_separator` is true for whitespace, `'\0'`, and `,.()+-/*=~%<>[]{};:!&|^?`
- [ ] `highlight_row` sets `hl` to `render`'s size and colours every character by the rules above, using `kKeywords` and `kTypes`
- [ ] It returns whether the row ends inside a `/* */` comment, starting inside one when `in_comment` is true
- [ ] After a string's closing quote or a comment's `*/`, the next character counts as starting a word
