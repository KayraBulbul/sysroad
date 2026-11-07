**stat, not open.** `fs::file_size` and `fs::is_regular_file` are the
`stat(2)` system call: one lookup of the file's inode, which records its
type, size, owner and timestamps. Nothing is read, which is why GNU can
afford to do this before counting.

**Real wc looks at stdin too.** GNU calls `fstat` on stdin. If you run
`wc < a.txt`, stdin *is* a regular file and its size counts, so it prints
` 1  2 12`. The lessons' checks feed stdin from a file too, which is
why the spec simply says "stdin means at least 7".

**Why the one-column rule exists.** `wc -l < file` is the standard way to
get a bare number in a script: `n=$(wc -l < file)`. POSIX allows leading
blanks, but GNU drops them in this case so the output is just the digits.

**Widths can still be wrong.** The sizes are read before counting. If a
file grows in between (a log being written, say), the counts can end up
wider than the column, and GNU just prints them wider. Your
`format_counts` does the same, because `std::format` never truncates.
