**References into a vector.** `Row& row = e.rows[e.cy];` followed by
`e.rows.insert(...)` is a classic bug: if the vector has to grow, it
moves every row to a new buffer, and `row` points at freed memory.
ASan reports it as a heap-use-after-free. Phase 2's `Vector<T>` did
exactly this move.

**This is O(n) per keystroke.** Inserting in the middle of a line
shifts the rest of that line, and inserting a line shifts every line
pointer after it. For a file of a million lines, that's still only a
few milliseconds, and a terminal can't show you more than 60 frames a
second anyway. Editors that open gigabyte files use a piece table (VS
Code) or a rope (Zed, Xi), where an edit costs O(log n).

**Auto-indent.** Most editors copy the leading whitespace of the line
you split onto the new line. In `insert_newline` that's the run of
spaces and tabs at the start of `row.chars`, prepended to `rest`, with
`cx` set to its length. A small change worth trying.
