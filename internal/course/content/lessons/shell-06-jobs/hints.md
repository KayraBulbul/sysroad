`init_job_control` is 6.2.6's, working on `sh.tty`:

```cpp
for (int sig : {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU}) {
    signal(sig, SIG_IGN);
}
if (getpgrp() != getpid()) {
    setpgid(0, 0);
}
tcsetpgrp(sh.tty, getpgrp());
```
---
One function can record anything `waitpid` tells you about a job:

```cpp
static void update(Job& job, pid_t pid, int st) {
    // find pid's index i, then:
    if (WIFSTOPPED(st))        { job.stopped = true; job.stop_signal = WSTOPSIG(st); }
    else if (WIFCONTINUED(st)) { job.stopped = false; }
    else                       { job.ended[i] = true; job.statuses[i] = decode(st); }
}
```

Waiting for a foreground job: for each PID in order, loop on
`waitpid(pid, &st, WUNTRACED)` and `update` until it has ended or the
job has stopped.
---
`run_foreground` and `fg` share the dance around the wait:
`tcgetattr` the shell's modes, `tcsetpgrp(sh.tty, job.pgid)`, (for
`fg`: `kill(-job.pgid, SIGCONT)`), wait, `tcsetpgrp(sh.tty,
getpgrp())`, `tcsetattr`. Only when `sh.interactive`.
---
A job that stops in `fg` already has a number: keep it. Give it a new
one only if it's 0 (it stopped the first time it ran).
