`start` is your last two lessons in one: build the `char*` array,
`std::fflush(nullptr)`, `fork`. In the child, `execvp(args[0],
args.data())` searches `PATH` for you. If it returns, it failed:

```cpp
std::fprintf(stderr, "%s: %s\n", args[0], std::strerror(errno));
_exit(127);
```
---
```cpp
if (WIFEXITED(wstatus)) {
    s.kind = Status::Exited;
    s.code = WEXITSTATUS(wstatus);
} else if (WIFSIGNALED(wstatus)) {
    s.kind = Status::Signaled;
    s.code = WTERMSIG(wstatus);
    s.core_dumped = WCOREDUMP(wstatus);
}
```

`wait_for` loops while `waitpid` returns -1 with `errno == EINTR`, and
throws `std::system_error(errno, std::generic_category(), "waitpid")`
for any other error.
---
The signal numbers differ between Linux and the Mac (`SIGUSR1` is 10
on one and 30 on the other), so switch on the names from `<csignal>`,
never on numbers.
---
```cpp
for (;;) {
    int wstatus = 0;
    pid_t pid = waitpid(-1, &wstatus, WNOHANG);
    if (pid > 0)      { /* add it, and go round again */ }
    else if (pid == -1 && errno == EINTR) { continue; }
    else              { return done; }   // 0, or ECHILD
}
```
