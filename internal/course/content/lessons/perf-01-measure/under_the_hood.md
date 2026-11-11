**Why the first `memset` is slow.** `new char[256 MB]` only reserves
addresses. No memory backs them until you touch a page, and then the
CPU faults into the kernel, which finds a free 4 KB page (16 KB on
macOS), zeroes it and maps it. That happens up to 65,536 times on the
first pass (fewer when Linux hands out 2 MB huge pages) and never on
the second. The same thing makes the first request
to a fresh server slow, and it's why databases and game engines touch
their memory up front.

**What the clock is made of.** On x86-64, `steady_clock` ends in
`clock_gettime(CLOCK_MONOTONIC)`, which the kernel answers without a
system call through the vDSO, a page of kernel code mapped into every
process. It reads the CPU's time-stamp counter with `rdtsc` and scales
it to nanoseconds. On Apple Silicon it ends in `mach_absolute_time`,
which reads a 24 MHz counter, hence the 41.7 ns step.

**Turbo, cores, and why numbers drift.** Modern CPUs change their
clock speed with load and temperature, so the same loop can take 20%
longer after a minute of heavy work. Hybrid chips like Intel's 13th
and 14th gen and every Apple Silicon Mac also have fast and efficient
cores. The OS decides which one runs you, and that can change between
samples. On Linux `taskset -c 0 ./prog` pins a program to core 0. On
macOS you can't pin, which is one more reason to look at the spread.

**The empty function isn't free.** Calling through a
`std::function` is an indirect call the optimiser can't inline, so it
costs a nanosecond or so per call. That's the floor of this harness.
For timings near it, put the loop inside `f` and divide by its length
yourself, the way the `sum` rows do.

**Measurement bias is real.** Mytkowicz et al. (in Deeper) found that
adding unrelated environment variables, or linking files in a
different order, changed the speed of the same program by up to 10%
by moving code and the stack to different addresses. When a change
makes code 3% faster, be suspicious. When it makes it 3× faster, it
probably did.

**The rows at the bottom.** Summing a `std::vector` of a million ints
takes about 0.05 ms: the loop is vectorised and the prefetcher
streams the memory in. A `std::list` with its nodes allocated one
after another takes 30 times longer, because each step waits for the
pointer to the next node. Scatter the same nodes around the heap, as
happens to a long-lived program's lists, and every step is a cache
miss: over 1000 times slower than the vector. Inserting at the front
of a vector moves every element each time, so building 20,000
elements that way does 200 million moves; `push_back` then
`std::reverse` does under 100,000. Chapter 1 explains the first result, and
lesson 2.4.3 the second.
