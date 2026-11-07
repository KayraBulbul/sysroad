**Case-insensitive means locale-dependent, in real grep.** GNU grep
folds case using your locale, so `-i` matches `É` with `é` in a UTF-8
locale. `std::tolower` on single bytes only folds ASCII. Proper Unicode
case folding works on whole characters and even changes lengths: `ß`
folds to `ss`. That's why it's done with ICU or similar libraries, not by
hand.

**`-c` doesn't stop early, but `-q` and `-l` do.** `-q` (quiet: just the
exit status) and `-l` (list matching files) can stop reading a file at
its first match. On a big file that's the difference between reading 1 KB
and 1 GB.

**`-n` is cheaper than it looks.** GNU grep doesn't count lines it skips
over until it needs a number. It counts newlines in the skipped region
with a fast `memchr`-style scan only when a match is printed.

**Flags after the pattern.** `grep todo main.cpp -n` works in GNU grep:
getopt "permutes" the arguments so flags can go anywhere. POSIX (and
macOS's BSD grep) stop at the first non-flag. Use `--` to say "no more
flags", which is how you search for a pattern that starts with `-`:
`grep -- -n file`.
