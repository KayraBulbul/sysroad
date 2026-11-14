A layout that grows well: `kite.hpp` for what the tests use (it needs
`<termios.h>`), `terminal.hpp` and `terminal.cpp` for the raw-mode
class, and `main.cpp`. `raw_attrs` is four lines of bit masks and two
of `c_cc`. To clear flags, `&=` with the complement:
`t.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);`. On the Mac the flag
fields are `unsigned long`, so wrap the mask in
`static_cast<tcflag_t>(...)` to keep the compiler quiet.
---
The raw-mode class keeps the fd and the saved settings:

```cpp
class RawMode {
  public:
    explicit RawMode(int fd);   // isatty, tcgetattr into saved_, tcsetattr(raw_attrs(saved_))
    ~RawMode();                 // tcsetattr(fd_, TCSAFLUSH, &saved_)
    RawMode(const RawMode&) = delete;
    RawMode& operator=(const RawMode&) = delete;
  private:
    int fd_;
    termios saved_;
};
```

When `isatty(fd)` is false, throw a `std::runtime_error` from the
constructor. Copies are deleted for the reason `Fd`'s were: two objects
restoring the same terminal.
---
`main` is a `try` around the loop, and a `catch` that prints and
returns 1:

```cpp
try {
    RawMode raw(STDIN_FILENO);
    while (true) {
        char c = 0;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n == -1 && errno != EAGAIN && errno != EINTR) { /* throw */ }
        if (n != 1) continue;           // no key in the last 1/10 s
        // print the byte, then "\r\n" and std::flush
        if (c == ('q' & 0x1f)) break;
    }
} catch (const std::exception& e) {
    std::cerr << "kite: " << e.what() << '\n';
    return 1;
}
```

`raw` is destroyed on the way out of the `try`, by `break` or by an
exception, so the terminal is back to normal before the error prints.
