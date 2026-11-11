**How sampling works.** `perf` asks the kernel to interrupt the
program about 4,000 times a second by default. At each interrupt it
saves the instruction pointer and walks the stack through the frame
pointers: each frame starts with the caller's saved `rbp` (`x29` on
arm64) and return address, a linked list back to `main`. Without frame
pointers, `perf record --call-graph dwarf` copies a chunk of the stack
instead and unwinds it afterwards with the debug info, which works but
is much bigger and slower.

**Why the libc frames look odd.** Your distro's `libc.so` is built
without frame pointers and its symbols are stripped, so `memcmp` may
appear as a bare address, and its callers can go missing. Installing
debug symbols (or, on Arch, letting `debuginfod` fetch them) fills in
the names. Fedora and Ubuntu now build their whole distribution with
frame pointers so that profiles just work.

**`perf stat` reads hardware counters.** CPUs count events like
cycles, instructions retired, cache misses and branch misses in
special registers. Instructions divided by cycles (IPC) is a quick
health check: above 2 the CPU is busy doing useful work, below 0.5 it
is mostly waiting on memory. On a hybrid Intel chip you'll see each
counter twice, for the performance (`cpu_core`) and efficient
(`cpu_atom`) cores. Macs have the same counters, but only Instruments'
CPU Counters template reads them.

**Flame graphs.** `perf script | stackcollapse-perf.pl |
flamegraph.pl > out.svg` (from Brendan Gregg's FlameGraph scripts)
draws every stack as a tower: width is time, and the wide plateaus at
the top are where the program spends it. Instruments and Firefox's
profiler (which can load `perf` output) draw the same thing.

**The quadratic trap.** The original `count_words` was fine on the
small inputs it was surely tested with. Linear search through a
vector is often the fastest option for a few dozen items, because it
streams through contiguous memory. It's the growth that kills it:
twice the words and twice the distinct words means four times the
work. Accidentally quadratic code is one of the most common
performance bugs in real software, and a profiler on realistic input
is how it gets found.

**Profiling isn't benchmarking.** A profiler tells you *where* time
goes, with some overhead and some sampling noise. It doesn't tell you
whether a change made things faster. For that you go back to
measuring, as in lesson 5.1.1.
