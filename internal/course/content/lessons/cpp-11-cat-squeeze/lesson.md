---
title: "-s: squeezing blank lines"
base: cpp-10-cat-number
minutes: 10
files: [cat.cpp]
run:
  args: ["-sn", "gaps.txt"]
  files:
    gaps.txt: "top\n\n\n\n\nmiddle\n\n\nbottom\n"
---
`cat -s` turns any run of empty lines into a single one:

```text
$ printf 'top\n\n\n\nmiddle\n' | cat -s
top

middle
```

An **empty line** is one whose first byte is `'\n'`. A line holding only
spaces isn't empty. That's what `blank_run` in `CatState` is for: count
the empty lines in a row, and drop every one after the first.

Like the numbering, the run **carries across files**. If `a.txt` ends with
two empty lines and `b.txt` starts with three, the joined output has one.

Squeezed lines are skipped completely, so with `-sn` they don't use up a
number either.

## Assignment

- [ ] At the start of a line, count empty lines in a row in `st.blank_run`
- [ ] With `-s`, skip an empty line when the one before was empty too
- [ ] Reset the run when a line has something in it
