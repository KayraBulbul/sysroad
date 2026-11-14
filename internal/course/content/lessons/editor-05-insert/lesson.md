---
title: Inserting and deleting text
kind: capstone
project: kite
program: kite
minutes: 45
run:
  args: [demo/sample.cxx]
  interactive: true
---
Time to edit. Four operations cover everything you can type:

```text
a character   'x' at the cursor:       he|llo   ->  hex|llo
Enter         splits the line:         he|llo   ->  he
                                                    |llo
Backspace     deletes before it:       he|llo   ->  h|llo
              at a line's start, it joins the line onto the one above
Delete        deletes at the cursor:   he|llo   ->  he|lo
              at a line's end, it joins the next line on
```

Each one changes `chars`, so it has to update the `render` of every row
it touched, or the screen shows the old text. And each one leaves the
cursor where you'd expect: after the typed character, at the start of
the new line, or where two lines were joined.

Mind the line just past the end. Typing there adds the line first, so
that typing at the end of a file grows it, and Enter there adds an
empty line. Backspace and Delete there have nothing to delete. In an
empty file the cursor is on that line, so the first key you type
creates line 1.

`process_key` sends them the keys: Enter (13), Backspace (127, and
Ctrl-H, 8, which some terminals send for it), Delete, and the keys that
**type**: Tab, the printable characters from 32 to 126, and bytes from
128 to 255, which are pieces of UTF-8 characters like `é`. Other
control keys, like Ctrl-C, and the Unknown sequences type nothing.

Editing reuses everything from the last lessons: the view scrolls to
follow the cursor as you type past the edge or press Enter on the
bottom row. There's no saving yet, so experiment freely.

## Contract

Nothing new. The tests type at kite with `process_key` and read the
text back off the screen.

## Assignment

- [ ] A typed character goes in at the cursor, which moves past it. On the line past the end, the line is added first
- [ ] Enter splits the line at the cursor (on the line past the end it adds an empty line) and moves to column 0 of the next line
- [ ] Backspace removes the character before the cursor. At column 0 it joins the line onto the end of the one above, with the cursor at the join. At the very start of the file, or on the line past the end, it does nothing
- [ ] Delete removes the character at the cursor, and at a line's end joins the next line on. At the end of the text it does nothing
- [ ] Tab, printable characters and bytes above 127 type; other control keys and unknown sequences don't. The screen always shows the text as it now is, tabs included
