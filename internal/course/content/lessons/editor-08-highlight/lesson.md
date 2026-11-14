---
title: Syntax highlighting for C++
kind: capstone
project: kite
program: kite
minutes: 50
run:
  args: [demo/sample.cxx]
  interactive: true
deeper:
  - title: "Build Your Own Text Editor: syntax highlighting"
    url: https://viewsourcecode.org/snaptoken/kilo/07.syntaxHighlighting.html
  - title: "Tree-sitter"
    url: https://tree-sitter.github.io/tree-sitter/
---
The rest of C++: comments, keywords, types and the preprocessor. Each
gets its own kind of character and its own colour:

```text
int n = 42;   // the answer           t  type, green (32)       k  keyword, yellow (33)
tttnnnnnddnnncccccccccccccc            c  comment, cyan (36)     p  preprocessor, blue (34)
```

Your walk along the row grows. At each position, the first rule that
fits wins:

```text
//                  a comment to the end of the line
inside /* */        a comment, until */ (both characters included)
/*                  starts one
inside a string     String, as before
" or '              starts a string
a digit, . or letter  Number, as before
a keyword or type   as a whole word: at the start of a word, with a separator or the line's end after it
# first on a line   Preproc, and so is the rest of the line, except strings, numbers and comments
anything else       Normal
```

Only comment markers outside strings count, and only quotes outside
comments. After a comment's `*/`, as after a string, the next character
starts a word. The keywords are

```text
if else for while do switch case default break continue return goto
try catch throw new delete class struct union enum namespace using
template typename public private protected virtual override static
const constexpr inline nullptr true false
```

and the types `int long short char bool float double void auto
unsigned signed size_t`. A `#` is only the preprocessor's when nothing
but spaces and tabs come before it on its line, and keywords aren't
looked for on a preprocessor line.

**Comments that span lines.** A `/*` comment can run on for any number
of lines, so a row's colours depend on the rows above it. The walk
takes one more piece of state in, whether the row **starts** inside a
comment, and gives one out: whether it **ends** inside one. Colouring a
file is a walk down the rows, each starting where the last ended.

Editing makes this interesting. Type `/*` on line 10 and every line
below it turns into a comment, until the next `*/`, maybe thousands of
lines down. Colouring the whole rest of the file on every key would
work, but most edits don't open or close a comment. So when a row
changes, colour it, then go on to the next row only while the next
row's starting state is different from the one it was last coloured
with. Each row remembers how it ended, and how it started. When a row
starts the same as last time, it and every row after it are still
right, so stop there.

## Contract

Nothing new. The tests read the colours off the screen.

## Assignment

- [ ] Comments are cyan, keywords yellow, types green and preprocessor lines blue, by the rules above, alongside last lesson's numbers and strings
- [ ] Keywords and types count only as whole words
- [ ] A `/* */` comment carries on across lines, from opening the file onwards
- [ ] Typing or deleting a `/*` or `*/` recolours every line it affects, including lines off the screen
