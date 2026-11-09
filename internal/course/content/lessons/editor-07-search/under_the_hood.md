**How fast is `std::string::find`?** libstdc++ and libc++ look for the
first character with `memchr`, which is vectorised, then compare. It's
O(n·m) in the worst case, but text rarely triggers that. `grep` uses
Boyer-Moore style skipping, which looks at the *last* character of the
pattern first and can jump ahead by the whole pattern's length on a
mismatch. C++17 ships it as `std::boyer_moore_searcher`.

**Searching the whole file on every key** is O(file size) per
keystroke. That's fine for megabytes. Editors for huge files search in
the background and show results as they come.

**Regular expressions.** `std::regex` would give kite regex search in a
few lines, but it's notoriously slow, and its multiline handling is
awkward. Editors use engines like RE2, PCRE2 or Rust's `regex`, which
guarantee linear time.

**Wrapping, and telling you.** vim prints "search hit BOTTOM,
continuing at TOP" when it wraps, because a match above the cursor can
be a surprise. kite could compare the match with `from` to say the
same.
