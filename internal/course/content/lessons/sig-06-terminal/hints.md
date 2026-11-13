```cpp
for (int sig : {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU}) {
    signal(sig, SIG_IGN);
}
if (getpgrp() != getpid()) {
    setpgid(0, 0);
}
tcsetpgrp(tty, getpgrp());
```

The `if` matters: a session leader already leads its group, and
`setpgid` refuses to move it.
---
`launch_job` is 6.2.5's `launch` with three additions in the child,
right after its `setpgid`:

```cpp
if (foreground) {
    tcsetpgrp(tty, getpgrp());
}
for (int sig : {SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU, SIGCHLD, SIGPIPE}) {
    signal(sig, SIG_DFL);
}
```

and in the parent, after the first fork, `tcsetpgrp(tty, job.pgid)`
for a foreground job. Fill in `ended` and `statuses` as you push each
PID.
---
```cpp
job.stopped = false;
while (!job.done()) {
    int st = 0;
    pid_t pid = waitpid(-job.pgid, &st, WUNTRACED);
    // -1 with EINTR: again; any other -1: give up
    // find pid in job.pids; WIFSTOPPED(st) → stopped, WSTOPSIG(st), and return
    // otherwise mark it ended, with decode(st)
}
```
---
Both `wait_foreground` and a foreground `continue_job` do the same
dance, so give it one helper:

```cpp
termios modes{};
bool have = tcgetattr(tty, &modes) == 0;
tcsetpgrp(tty, job.pgid);
// (continue_job: kill(-job.pgid, SIGCONT) here)
wait_for_job(job);
tcsetpgrp(tty, getpgrp());
if (have) {
    tcsetattr(tty, TCSADRAIN, &modes);
}
```
