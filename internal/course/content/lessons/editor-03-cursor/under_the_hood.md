**The remembered column.** Move down from the end of a long line
through a short one, and kite forgets where you were: the next long
line puts the cursor at the short line's length. Most editors keep a
"wanted column" (vim calls it `curswant`) that's set by Left, Right
and typing, and only used by Up and Down. It's one more `int` in
`Editor`, and a nice change to try.

**Columns aren't characters.** `cx` counts bytes. For ASCII that's the
same thing, but `é` is two bytes in UTF-8 and `🙂` is four, so Right
would step into the middle of a character. And on screen, `漢` takes
two columns, while a combining accent takes none. Real editors step
by grapheme clusters and measure with `wcwidth`. kite stays with bytes
to keep the code small.

**Pages.** vim's Ctrl-F also scrolls the view, keeping the cursor in
place on the screen. kite moves the cursor, and the scrolling from the
next lesson follows it. The result looks about the same.
