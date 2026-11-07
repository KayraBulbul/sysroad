All of this goes inside `if (st.at_line_start)`, *before* the numbering,
so a skipped line never gets a number.
---
```cpp
if (ch == '\n') {
    ++st.blank_run;
    if (o.squeeze && st.blank_run > 1) continue;
} else {
    st.blank_run = 0;
}
```
`continue` skips the rest of the loop body, so the newline is never added
to `text`. `at_line_start` stays true, which is right: you're still at the
start of a line.
