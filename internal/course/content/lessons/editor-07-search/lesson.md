---
title: Incremental search
kind: capstone
project: kite
program: kite
minutes: 50
run:
  args: [demo/sample.cxx]
  interactive: true
---
Ctrl-F searches as you type. It asks
`Search (arrows: next/previous, Esc: cancel): ` in the message bar, and
every key you add to the query moves the cursor to the first match from
where you started, so the screen follows along. The arrow keys step to
the next match (Right, Down) or the one before (Left, Up), Enter stays
there, and Escape takes you back to where you were, with the view as
it was. With no match the cursor goes back to where it started, and
Enter then says `Not found`.

That's last lesson's prompt again, with one more thing: it has to hear
about **every** key, not just the answer at the end. Give the prompt a
second, optional callback that's called after each key that doesn't
end it, with the answer so far and the key. Save-as doesn't set it;
search does. The search remembers things between keys (where it
started, and the match it's on), and both of its callbacks need them,
so they share them, through a `std::shared_ptr` both lambdas capture.

The heart of it is finding. Search from a position, `from`, in a
direction. Forwards, it wants the first match at or after `from`,
reading the text in order: the rest of `from`'s line, then each line
below. Backwards, the last match at or before `from`. A match **at**
`from` counts, which is what typing needs: when "ca" matched here,
"cat" should still match here. To step past the current match, the
arrows search from one column beyond it, or one before it.

If nothing turns up before the end (or the start), the search **wraps
around** and carries on from the other end, the way every editor's
"find next" does. Only when there's no match anywhere is the answer
"nothing".

```text
the cat        "cat" from 1:3, forwards    ->  1:6
sat on a cat   "cat" from 1:7, forwards    ->  0:4  (wrapped)
```

Matches are exact (case matters) and don't cross lines. An empty query
matches nothing. Moving the cursor is enough to show the match: the
scrolling you wrote moves the view the least it can to show it.

## Contract

Nothing new. The tests press Ctrl-F, type queries and arrows, and
watch the cursor and the message bar.

## Assignment

- [ ] Ctrl-F asks `Search (arrows: next/previous, Esc: cancel): ` and searches after every key, from where the cursor was when it started
- [ ] Forwards: the first match at or after the start, in reading order; then, wrapping around, the first match in the text
- [ ] Right and Down go to the next match, Left and Up to the one before, wrapping around both ways
- [ ] With no match, the cursor goes back to where it started; Enter then says `Not found`
- [ ] Enter stays at the match; Escape puts the cursor and the view back as they were
