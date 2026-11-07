**ifstream calls open(2).** Under the class, opening is the `open` system
call. It returns a file descriptor (a small integer index into your
process's table of open files) or -1 with `errno` set. `ENOENT` is "No such
file or directory" and `EACCES` is "Permission denied". `strace -e
openat wc a.txt nope.txt` shows both calls.

**errno is per thread.** It used to be a global variable. Now it's a macro
that expands to something like `*__errno_location()`, so each thread has
its own copy. You'll see why that matters in the concurrency phase.

**stdout and stderr are separate.** Errors on stderr mean `wc *.txt >
counts.txt` still shows them on your terminal, and the file holds only
the counts. `2>/dev/null` silences them. The streams are file descriptors 1
and 2.

**Directories.** `wc somedir` prints a row of zeros and then `wc: somedir:
Is a directory`, because opening a directory read-only succeeds and only
the first `read` fails, with `EISDIR`. An `ifstream` hides that read error.
`cat` handles it properly in the next chapter.

**Why the total still prints.** GNU decides whether to print `total` by
counting arguments, not successful files. Scripts that parse wc's output
can rely on the shape staying the same.
