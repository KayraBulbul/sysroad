---
title: RAII — a file descriptor that closes itself
minutes: 20
files: [fd.cpp]
run:
  args: ["note.txt"]
  files:
    note.txt: "read with open(2) and read(2)\n"
deeper:
  - title: RAII (cppreference)
    url: https://en.cppreference.com/w/cpp/language/raii
  - title: read(2) man page
    url: https://man7.org/linux/man-pages/man2/read.2.html
---
In C, every `open` needs a `close` on **every** path out of the function:
the early returns, the error branches, all of them. Miss one and you leak a
file descriptor. Do it a few thousand times and `open` starts failing with
`EMFILE: Too many open files`.

C++ ties the cleanup to an object's lifetime instead. That's **RAII**
("resource acquisition is initialisation"): the constructor takes the
resource, the destructor gives it back, and the compiler runs the
destructor whenever the object goes out of scope, by `return`, by falling
off the end, or by an exception.

```cpp
{
    Fd fd = open_read("data.bin");
    if (!fd.valid()) return;     // nothing to close
    auto text = read_all(fd);
    if (text.empty()) return;    // ~Fd() closes it
}                                // ~Fd() closes it here too
```

`fd.hpp` declares the class. Its interesting lines:

```cpp
~Fd();                                // closes, if it holds an fd
Fd(const Fd&) = delete;               // no copies:
Fd& operator=(const Fd&) = delete;    // two owners would close twice
```

Copies are deleted because a copy would mean two objects closing the same
number. The second `close` either fails, or worse, closes a file someone
else opened in the meantime and got the same number. Moving ownership
from one `Fd` to another is phase 2 (move semantics). Here `release()`
hands the raw fd over by hand.

These are the POSIX calls, from `<fcntl.h>` and `<unistd.h>`:
`::open(path, O_RDONLY)` returns an fd or -1, `::read(fd, buf, n)`
returns the bytes read, 0 at end of file, or -1 on error, and
`::close(fd)`. The leading `::` means "the global one", not a member.

## Assignment

- [ ] The destructor closes the fd if it's valid
- [ ] `release` returns the fd and stops owning it; `reset` closes the old one and takes a new one
- [ ] `open_read` opens a file for reading (an invalid `Fd` on failure)
- [ ] `read_all` reads until end of file with `::read`
