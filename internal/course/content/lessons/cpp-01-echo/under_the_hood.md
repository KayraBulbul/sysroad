**Where argv lives.** When a program starts, `execve` copies the argument
and environment strings onto the top of the new process's stack, followed
by the `argv` pointer array (ending in a null pointer, so
`argv[argc] == nullptr`). Your `std::vector<std::string>` copies each one
again, into memory the strings own.

**Small strings don't allocate.** `std::string` keeps short text inside
the object itself: up to 15 characters with libstdc++ (Linux) and 22 with
libc++ (Mac). This is the *small string optimisation*. Most arguments never
touch the heap.

**GNU echo's flag parsing is odd.** An argument counts as options only if
*every* character after the `-` is one of `n`, `e` or `E`. So `-nnn` and
`-neE` are flags, but `-nx` is printed as text. POSIX leaves `-n` up to the
implementation, which is why shell scripts that need to be portable use
`printf` instead.

**Your version vs GNU.** You build one string and print it. GNU writes each
argument straight to stdout with `fputs`, so it never needs one big buffer.
For echo the difference doesn't matter. For tools that stream gigabytes,
like `cat` in chapter 1.2, it does.

**`'\n'` vs `std::endl`.** `std::endl` writes a newline *and flushes*. A
flush is a `write` system call, so `std::endl` in a loop is a classic
performance bug. Use `'\n'`.
