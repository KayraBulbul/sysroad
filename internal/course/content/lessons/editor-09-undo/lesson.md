---
title: Undo and redo
kind: capstone
project: kite
program: kite
minutes: 55
run:
  args: [demo/sample.cxx]
  interactive: true
deeper:
  - title: "Text Editor: Data Structures (Avery Laird)"
    url: https://www.averylaird.com/programming/the%20text%20editor/2017/09/30/the-piece-table
  - title: "vim's undo tree (:help undo-tree)"
    url: https://vimhelp.org/undo.txt.html#undo-tree
---
Ctrl-Z undoes, Ctrl-Y redoes. To undo an edit, kite has to know what
it was, so first, a change to code you've already written: every edit
becomes data.

**Two primitives.** Your four edits change the rows in four different
ways. Rebuild them on two functions that take any text, line breaks
included:

```text
insert_text(at, text)   puts text in at a position, splitting lines at each "\n";
                        returns where the text ends
erase_text(at, n)       removes n characters from a position, a line break counting as one
```

Typing `x` is `insert_text(cursor, "x")`, Enter is
`insert_text(cursor, "\n")`, Backspace erases the one character before
the cursor (a `"\n"` at a line's start), and Delete the one at it. Each
primitive keeps `render` and the colours of the rows it touched up to
date. Typing on the line past the end inserts `"\n"` and the character
at the end of the last line, which adds the line the way you'd expect,
and Enter there inserts just the `"\n"`.
An Enter in a file with no lines at all is the one edit that doesn't
fit, so it can stay as it was, without an undo.

**Edits as data.** Each edit is then described by an `Edit`: inserted
or erased, where, the text, and where the cursor was before. Each of
the four records its `Edit` as it happens. That's the whole history.

Undo is a **stack**. Undoing pops the newest edit and does its
opposite: an insert is undone by erasing the same text from the same
place, and an erase by inserting it back. Then the cursor goes back to
where it was before the edit. The undone edit goes on a second stack,
**redo**, and redo does the edit again, leaving the cursor just after
an insert or at the place of an erase. Any new edit empties redo: once
you've typed something else, the undone future is gone. Undo and redo
both change the text, so both mark the file modified, and with nothing
to undo or redo, kite says `Nothing to undo` or `Nothing to redo`.

For now each edit is a step of its own, so undoing a word takes a
Ctrl-Z per letter. The next lesson groups them.

## Contract

Nothing new. The tests edit, then press Ctrl-Z and Ctrl-Y and look at
the screen. They're written so that grouping edits into bigger steps,
next lesson, can't break them: they never count how many presses an
undo takes, apart from single edits.

## Assignment

- [ ] Rebuild your edits so that each one inserts or erases some text and is remembered: what, where, and where the cursor was before. No test sees this directly; undo depends on it
- [ ] Ctrl-Z undoes the newest edit and puts the cursor back where it was before it; with none, it says `Nothing to undo`
- [ ] Ctrl-Y redoes the newest undone edit, with the cursor after an insert or at an erase; with none, it says `Nothing to redo`
- [ ] A new edit empties redo; undo and redo mark the file modified, and keep the colours right
