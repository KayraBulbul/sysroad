**Why fd 10.** POSIX lets scripts name fds 0 to 9 in redirections
(`3>log`), so shells keep their own private fds at 10 and above.
bash saves fds this way for every redirected builtin, and uses 255
for the script it's reading. That's the fd you met in 6.1.5's leak
test: `ls -l /proc/$$/fd` in bash shows it.

**`FD_CLOEXEC`, and why it's a flag on the fd.** A saved copy that
isn't close-on-exec leaks into every command the shell runs while it's
open, which for a builtin like `cd` is none, but for a shell that runs
`exec` or forks inside a builtin is a real leak. Linux 2.6.23 added
`O_CLOEXEC` to `open` and `F_DUPFD_CLOEXEC` to `fcntl` so the flag is
set atomically: setting it with a second call leaves a moment where
another thread's `fork` and `exec` inherit the fd.

**Why builtins can't just run in a child.** A builtin that runs in a
child can't change the shell, which is the reason it's a builtin. bash
does run a redirected builtin in the shell, as conch does; some older
shells forked for a redirected `cd`, so `cd /tmp > /dev/null` silently
did nothing.
