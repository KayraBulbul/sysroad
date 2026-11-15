**Why `SIGHUP` and then `SIGCONT`.** A stopped process can't act on a
signal until it runs, so `SIGHUP` alone would sit pending forever. Sent
together, the process wakes up, finds `SIGHUP` waiting, and (by
default) dies. The kernel does the same on its own when a process group
is orphaned with stopped members: when its parent's session leader
goes, it gets `SIGHUP` and `SIGCONT` too.

**`%%`, `%+` and `%-`.** bash keeps a "current" job (the most recently
stopped or backgrounded, shown with a `+` in `jobs`) and a "previous"
one (`-`). `fg` with no argument picks the current one, which isn't
always the newest. `%vim` picks a job by the start of its command, and
`%?make` by any part.

**`disown` and `nohup`.** bash's `disown` takes a job out of the table,
so the shell won't hang it up on exit. `nohup` does it from the other
side: it starts a command with `SIGHUP` ignored, and output in
`nohup.out`, so closing the terminal doesn't end it.

**Why `WCONTINUED` matters.** Without it a shell never learns that a
job it thinks is stopped was continued by someone else (`kill -CONT`
from another terminal), and `jobs` would go on saying `Stopped` for a
job that's running.
