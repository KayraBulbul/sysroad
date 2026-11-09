---
title: "Saving, and the unsaved-changes flag"
kind: capstone
program: kite
minutes: 35
files: [save.cpp]
run:
  args: [notes.txt]
  interactive: true
deeper:
  - title: "Files are hard (Dan Luu)"
    url: https://danluu.com/file-consistency/
  - title: "rename(2) (man7.org)"
    url: https://man7.org/linux/man-pages/man2/rename.2.html
---
kite gets two bars under the text. The **status bar**, in inverted
colours, says what you're editing. The **message bar** under it shows
messages like "6 bytes written to disk" for five seconds.

```text
notes.txt - 12 lines (modified)                        4/12
6 bytes written to disk
```

The `(modified)` comes from `dirty`, which every edit now sets and a
save clears. It also guards quitting: with unsaved changes, Ctrl-Q
warns, and only quits if you press it three more times.

Saving looks easy, `std::ofstream out(name); out << text;`, but that
opens the file by **emptying** it first. If the program crashes, or the
disk fills up, halfway through writing, your file is now half a file.
The safe way is to write the whole text to a **new** file next to it,
then `std::rename` the new file over the old one. Within one
filesystem, a rename is atomic: anyone looking sees the old file or the
new one, never a mix.

The course does the rest: Ctrl-S calls your `save_file`, asking for a
name first if the file has none, and the bars are drawn from your
`status_bar`.

## Assignment

- [ ] `rows_to_string` joins the rows, each followed by `\n`
- [ ] `save_file` writes the text to a temporary file next to `e.filename`, renames it over `e.filename`, clears `dirty`, and sets the message to `N bytes written to disk`
- [ ] When the file can't be written or renamed, `save_file` removes the temporary file, sets a message starting `Can't save!`, leaves `dirty` alone, and returns false
- [ ] `status_bar` is exactly `screen_cols` wide: on the left, the name (or `[No Name]`), cut to 20 characters, then ` - N lines`, then ` (modified)` if dirty; on the right, `line/lines`, with the line counted from 1. When both don't fit, drop the right side, then cut the left
