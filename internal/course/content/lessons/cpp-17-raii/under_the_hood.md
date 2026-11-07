**An fd is an index.** The kernel keeps a table of open files for each
process, and the fd is your index into it. New fds always take the
*lowest* free number. That's why closing twice is dangerous: after the
first close, the next `open` anywhere in the program gets the same
number, and your second close shuts it. In threaded programs that's a
real and miserable bug.

**`ls /proc/self/fd`** on Linux lists a process's open fds. Pick a PID and
look at `/proc/<pid>/fd` to see what a program has open. `lsof -p <pid>`
works on both Linux and macOS. `ulimit -n` is the per-process cap,
usually 1024 on Linux by default, which is why leaks show up as
`EMFILE`.

**EINTR.** A blocking `read` can be interrupted by a signal before any
data arrives. It returns -1 with `errno == EINTR`, and you just try
again. Phase 6 (signals) is where you'll make that happen on purpose.

**The standard library is RAII all the way down.** `std::ifstream`
closes its file, `std::vector` frees its buffer, `std::lock_guard`
unlocks a mutex, and `std::unique_ptr` deletes its object. Once you have
destructors, "remember to clean up" becomes "write the destructor once".
