**C++ has bit-field syntax, and formats avoid it.** You can write
`struct Pixel { unsigned b : 5; unsigned g : 6; unsigned r : 5; };`
and let the compiler do the shifting. But the standard doesn't say
which end of the word the first field goes at, so the layout changes
between compilers and CPUs. Code that reads a file or a network packet
uses explicit shifts and masks, like you just did, so it means the same
thing everywhere.

**Flags as an enum.** The usual way to name single bits is
`enum Perm : unsigned { Read = 1u << 2, Write = 1u << 1, Exec = 1u << 0 };`.
Then `mode & Write` tests one and `Read | Exec` builds a set. The
`open()` system call's `O_RDONLY | O_CREAT | O_TRUNC` is exactly this.

**The other three permission bits.** Above the nine you handled sit
setuid (`04000`), setgid (`02000`) and the sticky bit (`01000`). `ls`
shows them by changing an `x` into `s` or `t`. `/tmp` is `drwxrwxrwt`:
anyone can create files there, and the sticky bit stops them deleting
each other's.

**Why green gets the extra bit.** 16 bits don't divide by three, and
the human eye is most sensitive to green, so that's where the spare
bit went.

**A shift is the cheapest instruction there is.** `x << 3` and `x >> 3`
are single-cycle operations, far cheaper than a multiply or a divide.
The compiler turns `x * 8` and unsigned `x / 8` into shifts for you,
so write whichever one says what you mean. You'll watch it do that in
lesson 4.2.6.
