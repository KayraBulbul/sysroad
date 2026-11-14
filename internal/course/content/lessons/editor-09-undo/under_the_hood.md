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

**Why every edit goes through two functions.** Undo is only as good as
the history is complete. With four edit functions that each change the
rows their own way, one forgotten `record` means an edit undo can't
see, and the next undo then puts text back in the wrong place. With
two primitives that every edit uses, the history can't miss one.
