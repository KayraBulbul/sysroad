**Why yes | head is fast and stops.** `yes` doesn't wait for `head`
to read. It writes until the pipe is full (64 KB of `y\n`), then
sleeps in `write`. `head` reads a chunk, prints three lines and exits.
That closes the last read end, so the kernel wakes `yes` with
`SIGPIPE`. Without the signal reset in the child, `yes` would get
`EPIPE` instead; GNU `yes` notices and exits with an error, but plenty
of programs ignore `write`'s return value and would spin forever.

**What shells do differently.** A shell puts all the commands in a
pipeline into one **process group** (next chapter), so Ctrl-C reaches
every one of them and `fg` resumes them together. Bash also runs the
last command in a subshell by default; with `shopt -s lastpipe`, the
last one runs in the shell itself, so `echo hi | read x` can set `x`.
zsh does that by default. `${PIPESTATUS[@]}` in bash is your vector of
statuses.

**Pipes in 1973.** Doug McIlroy pushed for "a way of coupling programs
like garden hose" for years before Ken Thompson added `pipe()` to
Unix in one night, and rewrote the tools to read stdin and write
stdout. The `|` notation came the next week. The 1978 Bell System
Technical Journal issue in the further reading is where the Unix
philosophy was first written down.

**Splicing.** Each byte through a pipe is copied twice: from the
writer into the kernel, and out to the reader. Linux's `splice` moves
data between a pipe and a file or socket without copying it to user
space at all, and `vmsplice` hands the kernel your pages. `pv` and
some fast `cat`s use them, which is how a pipe can move tens of GB/s.
