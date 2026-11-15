Getting ready for job control is 6.2.6's, on the terminal (fd 0):

```cpp
while (tcgetpgrp(sh.tty) != getpgrp()) {
    kill(-getpgrp(), SIGTTIN);
}
for (int sig : {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU}) {
    signal(sig, SIG_IGN);
}
if (getpgrp() != getpid()) {
    setpgid(0, 0);
}
tcsetpgrp(sh.tty, getpgrp());
```
---
Your job gains a process group (`pgid`), whether it's stopped and by
which signal, and room for saved terminal settings (a `termios`).
Waiting for a foreground job: for each PID in order, loop on
`waitpid(pid, &st, WUNTRACED)` until it has ended or the job has
stopped. `WIFSTOPPED(st)` and `WSTOPSIG(st)` say it stopped; anything
else is an ending, as before.
---
Around the wait, when there's job control: `tcgetattr` the shell's
settings, `tcsetpgrp(tty, job.pgid)`, wait, then (if the job stopped)
`tcgetattr` into the job, `tcsetpgrp(tty, getpgrp())`, and
`tcsetattr(tty, TCSADRAIN, &shells)`. Put it in a function: next
lesson's `fg` does the same dance.
---
Starting a job, in the parent after each `fork`:

```cpp
if (job.pgid == 0) {               // the first process leads the group
    job.pgid = pid;
    if (foreground) tcsetpgrp(tty, job.pgid);
}
setpgid(pid, job.pgid);
```

and in the child, first thing: `setpgid(0, pgid)` (0 for the first),
`tcsetpgrp(tty, getpgrp())` for a foreground job, then
`signal(sig, SIG_DFL)` for the five.
