One function can record anything `waitpid` tells you about a job:

```cpp
void update(Job& job, pid_t pid, int st) {
    // find pid's index i, then:
    if (WIFSTOPPED(st))        { job.stopped = true; job.stop_signal = WSTOPSIG(st); }
    else if (WIFCONTINUED(st)) { job.stopped = false; }
    else                       { job.ended[i] = true; job.statuses[i] = st; }
}
```

Both your foreground wait and your before-each-line check can use it.
---
`fg` is last lesson's foreground dance with one step added between
handing over the terminal and waiting: if the job saved its terminal
settings, `tcsetattr` them, then `kill(-job.pgid, SIGCONT)`. A flag on
your foreground function ("resume it") keeps it one function.
---
A job that stops under `fg` already has a number: keep it. Give it a
new one only if it's 0 (it stopped the first time it ran).
---
For `exit`, check before setting the shell to leave: look for a stopped
job; if there's one and you haven't warned, warn, remember that you
did, and return 1 without leaving.
