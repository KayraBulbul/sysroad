**Commands, not snapshots.** kite stores each change, not a copy of
the text before it. That's the *command pattern*, and it's why undo
costs almost nothing in memory. The other approach, snapshots, is
viable when copies are cheap: persistent data structures (ropes, or
Clojure-style trees) share everything that didn't change, so a
snapshot per edit costs a few pointers.

**Undo trees.** Undo twice, type something, and kite's redo stack is
gone. vim keeps it: its history is a **tree**, and `g-`/`g+` walk
through every state the file has ever been in, including the
branches you undid. Emacs gets a similar effect differently: undoing is
itself an edit that goes on the history, so nothing is ever lost (and
undoing an undo is a redo).

**Modified, or not?** kite marks the file changed after any undo. A
better rule: remember the undo stack's size when you save, and the
file is unmodified whenever the stack is back to that size with the
same steps. That's how most editors clear `(modified)` when you undo
back to the saved text.

**Collaborative editing.** Undo gets hard when two people edit at once:
undoing "my" insert has to account for edits that came after it.
Google Docs uses operational transformation, which rewrites each edit
against the ones that happened since. Zed and others use CRDTs, data
structures whose edits commute, so the order they arrive in doesn't
matter.

**You built an editor.** About a thousand lines: raw mode, a renderer,
scrolling, editing, atomic saves, search, highlighting and undo. Run
`./kite kite.cpp`... well, kite is spread over many files now, so pick
one, and edit kite with kite.
