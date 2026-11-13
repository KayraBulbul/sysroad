Write into `buf` through a little lambda that drops anything past
`size`:

```cpp
size_t len = 0;
auto put = [&](char c) { if (len < size) buf[len++] = c; };
```

Digits come out backwards (`n % 10` is the last one), so collect them
in a small array and `put` them in reverse.
---
`-LONG_MIN` doesn't fit in a `long`. Work with the *negative* of the
number instead, which always fits: for `v <= 0`, the last digit is
`'0' - v % 10` (in C++, `-7 % 10` is `-7`), then `v /= 10`.
---
The handler can only reach the write end through a global:

```cpp
static volatile int write_fd = -1;

static void to_pipe(int sig) {
    int saved = errno;
    unsigned char byte = static_cast<unsigned char>(sig);
    write(write_fd, &byte, 1);
    errno = saved;
}
```
---
```cpp
fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);  // file status flags
fcntl(fd, F_SETFD, FD_CLOEXEC);                       // fd flags: a different set
```

Keep each signal's old `struct sigaction` in `old_` for the
destructor. In `next`, a `read` that fails with `EAGAIN` means the
pipe is empty: return 0.
