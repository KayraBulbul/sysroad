---
title: Incremental search
base: editor-06-save
kind: capstone
program: kite
minutes: 30
files: [search.cpp]
run:
  args: [demo/sample.cpp]
  interactive: true
---
Ctrl-F searches as you type: every key you add to the query moves the
cursor to the first match after where you started, so the screen
follows along. The arrow keys step to the next or previous match,
Enter stays there, and Escape takes you back to where you were. The
course has built that loop. It calls your `find` after every key.

`find` searches from a position, `from`, in a direction. Forwards
(`dir = 1`), it wants the first match at or after `from`, reading
the text in order: the rest of `from`'s line, then each line below.
Backwards (`dir = -1`), the last match at or before `from`. A match
**at** `from` counts, which is what typing needs: when "ca" matched
here, "cat" should still match here. To step past the current match,
the arrows call `find` with the column one beyond it, or one before it.

If nothing turns up before the end (or the start), the search **wraps
around** and carries on from the other end, the way every editor's
"find next" does. Only when there's no match anywhere is the answer
"nothing".

```text
the cat        find("cat", from 1:3, dir 1)   ->  1:6
sat on a cat   find("cat", from 1:7, dir 1)   ->  0:4  (wrapped)
```

## Assignment

- [ ] Forwards: the first match at or after `from`, in reading order; then, wrapping around, the first match in the text
- [ ] Backwards: the last match at or before `from`; then, wrapping around, the last match in the text
- [ ] `from` may be past the end of its line, before its start (column -1), or on the line past the end of the text
- [ ] An empty query, or no text at all, finds nothing. Matches are exact (case matters) and stay within one line
