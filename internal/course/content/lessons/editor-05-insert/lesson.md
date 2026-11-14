---
title: Inserting and deleting text
base: editor-04-open
kind: capstone
program: kite
minutes: 30
files: [edit.cpp]
run:
  args: [demo/sample.cpp]
  interactive: true
---
Time to edit. Three operations cover everything you can type:

```text
insert_char      'x' at the cursor:       he|llo   ->  hex|llo
insert_newline   Enter splits the line:   he|llo   ->  he
                                                       |llo
delete_char      Backspace:               he|llo   ->  h|llo
                 at a line's start, it joins it to the line above
```

Each one changes `chars`, so it has to call `update_render` on every
row it touched, or the screen shows the old text. And each one leaves
the cursor where you'd expect it: after the typed character, at the
start of the new line, or where the two lines were joined.

Mind the line just past the end. Typing there first adds the line, so
that typing at the end of a file grows it. Backspace there has nothing
to delete. In an empty file, the cursor is on that line, so the first
key you type creates line 1.

The course's `process_key` (in `editor.cpp`) wires up the keys:
printable characters and Tab insert, Enter, Backspace, and Delete,
which is Backspace one position to the right. There's no saving yet,
so experiment freely.

## Assignment

- [ ] `insert_char` inserts at `cx` on line `cy` and moves the cursor right. On the line past the end, it adds the line first
- [ ] `insert_newline` splits the line at the cursor (the line past the end gets an empty line added) and moves to column 0 of the next line
- [ ] `delete_char` removes the character before the cursor. At column 0 it joins the line onto the end of the one above, with the cursor at the join. At the very start of the file, or on the line past the end, it does nothing
- [ ] Every changed row's `render` is up to date
