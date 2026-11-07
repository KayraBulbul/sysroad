**`out << in.rdbuf()` is the one-liner,** and it does copy bytes exactly.
It has a trap, though: if `in` is empty, nothing gets inserted and the
standard says `out`'s **failbit** is set. Every later write to `out` then
silently does nothing. `cat empty.txt real.txt` would print nothing. The
explicit loop doesn't have that problem.

**Why 64 KiB.** Each `read` and `write` underneath is a system call, which
costs hundreds of nanoseconds. Above about 64–128 KiB per call, the copy
cost dominates and a bigger buffer stops helping. Pipes on Linux hold 64
KiB by default, so that's also how much a reader can take in one go. The
performance phase measures this properly.

**`std::array` lives on the stack.** 64 KiB is fine. The default main-thread
stack is 8 MiB on Linux and macOS. Make it 16 MiB and you'll crash with a
stack overflow before `main`'s first line.

**Text mode is a Windows thing.** Without `std::ios::binary`, Windows turns
`\r\n` into `\n` as you read and back as you write. On Linux and macOS
there's no difference. Binary is still the honest choice for a tool that
mustn't change anything.
