---
title: "Saving, and the unsaved-changes flag"
kind: capstone
project: kite
program: kite
minutes: 55
run:
  args: [notes.txt]
  interactive: true
deeper:
  - title: "Files are hard (Dan Luu)"
    url: https://danluu.com/file-consistency/
  - title: "rename(2) (man7.org)"
    url: https://man7.org/linux/man-pages/man2/rename.2.html
---
Ctrl-S saves. Three things come with it.

**The unsaved-changes flag.** Every edit that changes the text sets a
`dirty` flag, and a save clears it. The status bar shows it:

```text
notes.txt - 12 lines (modified)                        4/12
12 bytes written to disk
```

A key that changes nothing, like Backspace at the start of the file,
leaves it alone. The flag also guards quitting: with unsaved changes,
Ctrl-Q doesn't quit but warns, `Unsaved changes! Press Ctrl-Q 3 more
times to quit anyway.`, and counts down with each Ctrl-Q (`2 more
times`, `1 more time`) until the fourth quits. Any other key starts
the count again.

**Saving safely.** `std::ofstream out(name); out << text;` looks like
enough, but it opens the file by **emptying** it first. If the program
crashes, or the disk fills up, halfway through writing, your file is
now half a file. The safe way is to write the whole text to a **new**
file next to it, then `std::rename` the new file over the old one.
Within one filesystem a rename is atomic: anyone looking sees the old
file or the new one, never a mix. The text is every line followed by
`\n`. Then `N bytes written to disk`, or if anything fails,
`Can't save!` and the reason, with the temporary file removed and the
file still marked modified.

**Asking for a name.** A file with no name needs one first: Ctrl-S
asks `Save as: ` in the message bar, and what you type shows after it.
Backspace (or Delete) removes the last character, Enter with an answer
saves under that name, and Escape gives up with `Not saved`.

The obvious way to ask is a loop inside the Ctrl-S code that reads keys
and redraws until Enter. kite does it differently: the question is part
of the editor's **state**. Ctrl-S stores a prompt in the `Editor` (a
label, the answer so far, and what to do with the answer) and returns.
While a prompt is there, `process_key` sends every key to it, and
`draw_screen` shows it in the message bar instead of the message.
Nothing in `main` changes, and the prompt can't get out of step with
the screen, because `main` still draws after every key. "What to do
with the answer" is a function stored in the prompt, a lambda in a
`std::function` (lesson 1.4.3), so next lesson's search can ask its own
question the same way.

## Contract

Nothing new. The tests press Ctrl-S, then read the file back from disk
and the bars off the screen.

## Assignment

- [ ] After a change to the text, the status bar shows ` (modified)` after the line count until the file is saved; keys that change nothing don't
- [ ] Ctrl-S writes every line followed by `\n` to a temporary file next to the file and renames it over the file; then it says `N bytes written to disk` and ` (modified)` goes
- [ ] When it can't write or rename, it removes the temporary file, says `Can't save!` and why, and the file stays ` (modified)`
- [ ] With no name, Ctrl-S asks `Save as: ` in the message bar; Enter saves under the answer, Escape says `Not saved`, and while it asks, every key goes to the answer
- [ ] With unsaved changes, Ctrl-Q warns and counts down from 3, and any other key starts the count again
