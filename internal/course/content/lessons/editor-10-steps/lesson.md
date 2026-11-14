---
title: Undo a word at a time
kind: capstone
project: kite
program: kite
minutes: 40
run:
  args: [demo/sample.cxx]
  interactive: true
---
One keystroke per undo step is tedious: undoing a sentence takes a
Ctrl-Z per letter. Real editors undo in **steps** that match what you
did, and so will kite. `record` **joins** an edit onto the newest step
when it carries straight on from it, instead of pushing a new one:

```text
typing "hello world"     two steps: "hello " and "world" (a word after a space starts a new one)
Backspace x3             one step: the erased text grows to the left
Delete x3                one step: it grows to the right
Enter                    its own step: nothing with a line break joins
```

"Carries straight on" means: the same kind (typing doesn't join
erasing), on the same line, with no line break in either, and starting
exactly where the last one left off. Typing at column 5 carries on from
a step that inserted 3 characters at column 2; a Backspace at column 4
carries on from an erase that started at column 5; a Delete carries on
from an erase at its own column. When a Backspace joins, the step's
text and position move left, but the cursor it remembers stays where
it was before the *first* Backspace, which is where undo should put
it.

Lining up isn't always enough. Type `ab`, move left and right again,
and type `cd`: it carries on from `ab` exactly, but you moved in
between, and most people expect two steps. So the history has a
**sealed** flag: moving the cursor (the arrows, Home, End, the page
keys, or a search) seals it, and so do undo and redo. The next edit
after a seal starts a new step whatever it is, and unseals.

## Contract

Nothing new. The tests count Ctrl-Zs now, and check where each one
leaves the text and the cursor.

## Assignment

- [ ] Typing, Backspaces and Deletes that carry straight on from the edit before, by the rules above, undo and redo as one step; Enter, and any edit with a line break, is a step of its own
- [ ] Undoing a run of Backspaces or Deletes puts the cursor back where it was before the first of them
- [ ] Moving the cursor, a search, undo and redo end a step: the next edit starts a new one, even if it lines up with the last
