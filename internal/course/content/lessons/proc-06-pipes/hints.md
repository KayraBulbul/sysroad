A helper that both functions use saves writing it twice:

```cpp
// in_fd/out_fd become the child's 0 and 1 (-1: leave it alone);
// parent_ends are fds the child must close.
static pid_t start_with(const std::vector<std::string>& argv, int in_fd, int out_fd,
                        std::vector<int> parent_ends);
```

In the child: `dup2` each one into place, close the originals and the
parent's ends, `signal(SIGPIPE, SIG_DFL)`, then `execvp`.
---
For `capture`, the parent's half:

```cpp
close(p[1]);                      // now read() can see the end
char buf[65536];
for (;;) {
    ssize_t n = read(p[0], buf, sizeof buf);
    if (n > 0) out.append(buf, n);
    else if (n == -1 && errno == EINTR) continue;
    else break;
}
close(p[0]);
status = wait_for(pid);           // only now
```
---
For `communicate`, keep `size_t sent` and loop while you're still
reading or still writing. Each time round, put only the fds you still
need in the `pollfd` array. When the write end is ready, write from
`input.data() + sent`; on `EAGAIN` or `EINTR` try again later, and on
any other error (`EPIPE`) give up writing.
---
```cpp
struct sigaction ignore{}, old{};
ignore.sa_handler = SIG_IGN;
sigemptyset(&ignore.sa_mask);
sigaction(SIGPIPE, &ignore, &old);
// ... the poll loop ...
sigaction(SIGPIPE, &old, nullptr);
```

An empty input still needs the write end closed straight away, or
`cat` waits for input that never comes.
