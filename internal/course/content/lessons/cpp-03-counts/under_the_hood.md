**GNU wc reads much bigger chunks.** coreutils uses `read(2)` with a buffer
of its `IO_BUFSIZE`, which is 128–256 KiB depending on the version. Every
`read` is a system call, so bigger chunks mean fewer of them. You can see
the calls with `strace -e read wc somefile` on the PC.

**`wc -c` often doesn't read at all.** If you only ask for bytes and the
input is a regular file, GNU wc gets the size from `fstat` and skips the
reading entirely. That's why `wc -c` on a 10 GB file is instant. Pipes have
no size, so they still have to be read.

**iostreams add a layer.** `std::cin.read` goes through a `streambuf`. By
default `std::cin` is also synchronised with C's `stdin`, which makes it
slower. Fast C++ programs call `std::ios::sync_with_stdio(false);` at the
start of `main`. You'll measure the difference in the performance phase.

**Why `'\n'` and not "lines".** POSIX defines a line as text *ending* in a
newline, so `printf 'a' | wc -l` prints `0`. wc counts newline bytes, which
is also why counting lines is so easy to speed up.

**Columns.** GNU wc picks the column width from the largest count it might
print, which for stdin is 7. With file arguments it uses the file sizes.
You'll match that exactly in lesson 1.1.7.
