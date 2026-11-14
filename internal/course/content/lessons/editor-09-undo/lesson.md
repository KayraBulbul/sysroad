---
title: Undo and redo
base: editor-08-highlight
kind: capstone
program: kite
minutes: 40
files: [undo.cpp]
run:
  args: [demo/sample.cpp]
  interactive: true
deeper:
  - title: "Text Editor: Data Structures (Avery Laird)"
    url: https://www.averylaird.com/programming/the%20text%20editor/2017/09/30/the-piece-table
  - title: "vim's undo tree (:help undo-tree)"
    url: https://vimhelp.org/undo.txt.html#undo-tree
---
The last feature: Ctrl-Z undoes, Ctrl-Y redoes. The course has rebuilt
the edits on two primitives, `insert_text` and `erase_text`, which take
any text, line breaks included. Each edit is then described by an
`Edit`: inserted or erased, where, what text, and where the cursor was
before. The edits hand every `Edit` to your `record`.

Undo is a **stack**. Undoing pops the newest edit and does its
opposite: an insert is undone by erasing the same text from the same
place, and an erase by inserting it back. Then the cursor goes back to
where it was before the edit. The undone edit goes on a second stack,
**redo**, and redo does the edit again. Any new edit empties redo:
once you've typed something else, the undone future is gone.

One keystroke per undo step would be tedious, so `record` **joins**
an edit onto the last one when it carries straight on from it:

```text
typing "hello world"     two steps: "hello " and "world" (a word after a space starts a new step)
Backspace x3             one step: the erased text grows to the left
Delete x3                one step: it grows to the right
Enter                    its own step: nothing with a line break joins
```

The edit sets `sealed` after you move the cursor, and undo and redo
set it too: the next edit starts a fresh step even if it happens to
line up with the last.

## Assignment

- [ ] `record` empties redo, then joins the edit onto the newest step if it carries on from it (by the rules above, on the same line, and only when not `sealed`), or pushes it as a new step. Either way, it unseals
- [ ] `undo` reverses the newest step with `erase_text` or `insert_text`, puts the cursor at the step's `cursor`, marks the file dirty, moves the step to redo, and seals. It returns false when there's nothing to undo
- [ ] `redo` does the newest undone step again, puts the cursor just after an insert or at the place of an erase, marks the file dirty, moves the step back to undo, and seals. It returns false when there's nothing to redo
