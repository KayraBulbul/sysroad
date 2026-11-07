`release` swaps the fd out: `int fd = fd_; fd_ = -1; return fd;`. Now the
destructor sees -1 and does nothing.
---
`reset(int fd)`: `if (fd_ >= 0) ::close(fd_); fd_ = fd;`. Watch for
`reset(get())`, which would close the fd you're about to keep. Check
`fd != fd_` first.
---
```cpp
std::string out;
char buf[4096];
for (;;) {
    ssize_t n = ::read(fd.get(), buf, sizeof buf);
    if (n > 0) out.append(buf, n);
    else if (n == 0) break;               // end of file
    else if (errno != EINTR) break;       // a real error
}
```
