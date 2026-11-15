A layout that keeps the terminal out of the logic: a line-editor class
with the text, the cursor, and a pointer to the history, with
`feed(key)` returning what happened (still editing, done, cancelled,
end of input) and `render(prompt)` returning what to write; then a
function that puts the terminal in raw mode (an RAII class, as in kite)
and loops: read a key, feed it, write the render.
---
`feed` is one `switch` on the key. `std::string::insert(cursor, 1, c)`
and `erase(pos, n)` do the editing; after an insert, move the cursor
right. For Ctrl-W, step left from the cursor over spaces, then over
non-spaces, and erase from there to the cursor.
---
History browsing needs two more members: which history line is shown
(`SIZE_MAX` while you're on the new line), and the new line itself,
saved. The first Up saves the text and starts at `history.size()`;
each Up moves one older; Down moves newer, and past the end puts the
saved text back.
---
```cpp
std::string out = "\r" + prompt + text + "\x1b[K";
if (cursor < text.size()) {
    out += "\x1b[" + std::to_string(text.size() - cursor) + "D";
}
```

Waiting for the rest of an escape sequence, with `VMIN` 1: while what's
pending decodes as Escape and could still become a sequence (a lone
ESC, or `ESC [` or `ESC O` and not yet long), `poll` the fd for 50 ms;
if something came, read it and decode again; if not, it's Escape.

```cpp
bool could_be_sequence(const std::string& b) {
    return b.size() == 1 || (b.size() < 8 && (b[1] == '[' || (b[1] == 'O' && b.size() == 2)));
}
```
