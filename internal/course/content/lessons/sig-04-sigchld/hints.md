Every public function needs `SIGCHLD` blocked for a while, so a small
helper pays off:

```cpp
static sigset_t only_sigchld() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    return set;
}
// sigprocmask(SIG_BLOCK, &set, &old) ... sigprocmask(SIG_SETMASK, &old, nullptr)
```
---
`collect` walks the arrays up to `reaped_count`, erasing each PID
from `running_` and pushing `{pid, decode(status)}` onto `done_`,
then sets `reaped_count = 0`. That's only safe because the handler
can't run in the middle.
---
In `start`, block first, then fork. In the child, `sigprocmask(
SIG_SETMASK, &old, nullptr)` before `execvp`. In the parent, insert
the PID, then restore the mask; a `SIGCHLD` that arrived meanwhile is
delivered right then, and finds the PID in place.
---
```cpp
sigset_t during = old;          // what the caller had...
sigdelset(&during, SIGCHLD);    // ...but let SIGCHLD in
collect();
while (!running_.empty()) {
    sigsuspend(&during);
    collect();
}
```
