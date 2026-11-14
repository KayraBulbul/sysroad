**Where to cut steps.** kite's rule, a word at a time, is roughly
what most editors do. Vim makes everything from entering insert mode
to leaving it one step, however long you typed. VS Code cuts on
words and also when you pause. Emacs groups about twenty plain
keystrokes into one undo. None is "right": a step should be what you'd
want back in one go, and that depends on how you work.

**Joining in place.** Growing the newest `Edit` instead of keeping a
list of tiny ones keeps the history small: a page of typing is a few
hundred steps, not thousands of one-character edits. It's also why
`record` is the only place that knows about steps: `undo` and `redo`
take back whatever an `Edit` says, one character or a paragraph.

**Sealing** is an old trick for anything that merges events, not just
undo: an IDE's "go back" merges small cursor moves the same way, and
a database's write-ahead log groups writes into transactions with a
commit that plays the part of the seal.

**You built an editor.** About a thousand lines, every one of them
yours: raw mode, a key decoder, a renderer, scrolling, editing, atomic
saves, a prompt, search, highlighting and undo. Open one of your own
files with it, `kite render.cpp` say, and edit kite with kite.
