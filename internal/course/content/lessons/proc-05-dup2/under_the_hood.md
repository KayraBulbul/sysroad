**Close-on-exec.** Every fd a process has open is inherited by the
programs it `exec`s, unless it's marked **close-on-exec**:
`open(path, flags | O_CLOEXEC)`, or `fcntl(fd, F_SETFD, FD_CLOEXEC)`
afterwards. Without it, a long-running server that starts helper
programs hands them every socket and file it has open, which is a
security hole and a classic cause of "why is this port still in use".
Go and Python open everything with `O_CLOEXEC` and pass only the fds
you ask for. `dup2` deliberately *clears* the flag on the new fd, so
you can open with `O_CLOEXEC` and still redirect.

**Why the separate step at all.** A process could have asked the
kernel to start a program with given files on given fds, the way
Windows' `CreateProcess` takes handles. Unix instead lets the child
run your own code between `fork` and `exec`, with any system call
available. Redirection, `cd`, `nice`, `ulimit`, `chroot` and dropping
privileges all fall out of that one design, with no extra API.
`posix_spawn` has to bring it back as a list of "file actions" to
replay in the child.

**Shared offsets in action.** `(echo one; echo two) > f` works because
both `echo`s share one open file and its offset: the second writes
after the first. If each opened `f` itself, the second would write at
offset 0 over the first. `O_APPEND` goes further: every write moves to
the current end of the file first, as one atomic step, so several
processes can append to one log without losing lines.

**Where these live in the kernel.** On Linux, your fd table is a
`files_struct` holding an array of pointers to `struct file`, the open
file, which points at the inode (through a `dentry`). `ls -l
/proc/self/fd` shows what each of a process's fds points to, and
`/proc/self/fdinfo/1` shows its offset and flags.

**Here-documents.** `cat <<EOF` is a redirection too: the shell
writes the text to a temporary file or a pipe, and makes that fd 0.
`2>&-` closes fd 2, and `exec 3>log` with no command applies the
redirection to the shell itself, so every later command inherits it.
