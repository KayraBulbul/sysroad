**GNU counts in text, not in an int.** cat keeps its line number as a
string of digits (`line_buf`) and increments it by hand, carrying from
the last digit like on paper. That way it never has to convert the
number to text for each line, and it can't overflow. Past 999999 the field
just grows, which your `{:>6}` does too.

**Why a tab?** Six digits and a tab means that for anything up to a
million lines, the text starts at column 8, the next tab stop, so tabs
inside lines still line up. `nl` (number lines) is the more flexible tool,
with its own width (`-w`), separator (`-s`) and numbering style.

**One byte at a time sounds slow,** and it is slower than `copy_stream`,
but the loop body is a few instructions and the compiler keeps `ch` in a
register. GNU's flag-handling loop works the same way, with a sentinel
newline at the end of its buffer to avoid one bounds check per byte.
That's why `plain()` exists: without flags, skip the loop completely.
