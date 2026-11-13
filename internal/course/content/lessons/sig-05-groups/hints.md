Start from your `run_pipeline`. The group's ID isn't known until the
first fork returns, so keep `job.pgid` at 0 until then: in the child,
`setpgid(0, job.pgid)` with 0 means "lead a new group", and with the
first PID means "join it".

```cpp
// child, before the dup2s and exec:
setpgid(0, job.pgid);
// parent, after each fork:
if (job.pgid == 0) {
    job.pgid = pid;
}
setpgid(pid, job.pgid);
```
---
`kill(-job.pgid, sig)` signals the whole group; throw
`std::system_error(errno, std::generic_category(), "kill")` if it
returns -1.
---
`start_detached` is `start` with one more line in the child, before
`execvp`: `setsid();`. It fails only if the process already leads a
group, which a fresh child never does.
