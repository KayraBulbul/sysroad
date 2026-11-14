Two more members do it: `bool dirty = false;` and a quit counter,
`int quit_times = 3;`. Set `dirty` in each edit, at the point where it
really changes something (not in the early `return`s). In the status
bar, the left side becomes
`std::format("{:.20} - {} lines{}", name, e.rows.size(), e.dirty ? " (modified)" : "")`.
---
Writing: open a `std::ofstream` on `e.filename + ".kite-save"` with
`std::ios::binary | std::ios::trunc`, write the text, and `close()` it
before checking `if (!out)`: errors from flushing the last bytes only
show up on close. `std::rename(from, to)` returns 0 on success. On
failure, `errno` says why, and `std::strerror(errno)` turns it into
words like "No such file or directory". Read `errno` straight away:
other calls, like the `std::remove` cleanup, can overwrite it.
---
A prompt is a small struct, and the editor holds one or none:

```cpp
struct Prompt {
    std::string label;      // "Save as: "
    std::string answer;
    std::function<void(Editor&, std::optional<std::string>)> done;  // nullopt: Escape
};
// in Editor:
std::optional<Prompt> prompt;
```

It mentions `Editor` before `Editor` is defined, so declare
`struct Editor;` above it. In `draw_screen`, the message bar shows
`label + answer` while `e.prompt` holds one.
---
At the top of `process_key`, before the `switch`:

```cpp
if (e.prompt) {
    // Escape, or Enter with an answer: take the prompt out, then call done
    // Backspace, Ctrl-H or Delete: answer.pop_back(), if there's one
    // a typing key (32 to 255, not 127): answer += it
    return true;
}
```

Take it out of the editor (`Prompt p = std::move(*e.prompt);
e.prompt.reset();`) before calling `p.done`, so `done` is free to ask
something new. Ctrl-S without a name sets
`e.prompt = Prompt{"Save as: ", "", [](Editor& e, std::optional<std::string> name) { ... }};`.
---
The Ctrl-Q countdown, with the reset for every other key before the
`switch`:

```cpp
if (key != ctrl_key('q')) e.quit_times = 3;
...
case ctrl_key('q'):
    if (e.dirty && e.quit_times > 0) {
        set_status(e, std::format("Unsaved changes! Press Ctrl-Q {} more time{} to quit anyway.",
                                  e.quit_times, e.quit_times == 1 ? "" : "s"));
        --e.quit_times;
        return true;
    }
    return false;
```
