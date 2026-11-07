**GNU uses getopt.** coreutils parses flags with `getopt_long`, the C
library's standard parser. It handles bundling (`-lw`), long options
(`--lines`), `--` (end of flags), and "permutation": flags can come after
file names, as in `wc file.txt -l`. Your loop already allows that, since it
looks at every argument.

**`string_view` is a pointer and a length.** On 64-bit machines it's 16
bytes and it never owns memory. `args` holds views into the strings the
kernel copied onto the top of your process's stack at `execve` time, so
they're valid for the whole of `main`.

**Why `char bad` and not an exception?** A bad flag is an expected input,
not a broken invariant. Returning it as data keeps the parser easy to test.
Chapter 1.4 compares the options properly: `std::optional`, error values
and exceptions.

**Exit codes.** `main`'s return value becomes the process's exit status.
The shell shows it as `$?`. GNU tools use 0 for success and 1 for "bad
usage or failed input". grep is the odd one out: 1 means "no match", and 2
means an error.
