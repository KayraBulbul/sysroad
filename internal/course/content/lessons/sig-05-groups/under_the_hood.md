**Why both calls are safe.** `setpgid` only works on yourself or on
a child that hasn't `exec`'d yet, and only within your session. Once
a child has `exec`'d, its group is fixed from the outside, which is
why the parent's late call gets `EACCES` instead of moving a running
program around.

**Orphaned process groups.** If a group's processes have no parent
left in the same session but outside the group (say, the shell exited
while a job was stopped), the group is *orphaned*. A stopped,
orphaned group would wait forever for a `fg` that can't come, so the
kernel sends it `SIGHUP` then `SIGCONT`. Most programs die of the
`SIGHUP`, which is the point.

**SIGHUP, and nohup.** When the terminal goes away (you close the
window, or SSH drops), the kernel sends `SIGHUP` to the session
leader, and shells pass it on to their jobs. `nohup cmd` sets
`SIGHUP` to ignored before exec'ing `cmd`; `setsid cmd` (the command)
moves it to a new session with no terminal to lose. `tmux` keeps
programs alive by being their terminal itself, in a server process
that doesn't care whether you're attached.

**Daemons.** The classic recipe: fork and let the parent exit (so the
child isn't a group leader), `setsid`, fork *again* (so the daemon
isn't a session leader and can never acquire a terminal by opening
one), `chdir("/")`, `umask(0)`, and point 0, 1 and 2 at `/dev/null`.
systemd and launchd made most of this unnecessary: they start the
program detached already and collect its output.

**Killing a whole tree.** Process groups are how `timeout`, `make -j`
and test runners kill everything a command started, and it's how
sysroad's own runner kills your program and anything it left behind
when a test times out (`Setpgid: true` in its Go code). Linux cgroups
go further: a group no process can leave, which is how systemd stops
a service without missing a double-forked daemon.
