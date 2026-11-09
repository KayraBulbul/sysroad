---
title: Benchmark yours against the standard library
minutes: 25
files: [bench.cpp]
optimize: true
run: {}
deeper:
  - title: "CppCon 2015: Chandler Carruth, Tuning C++: Benchmarks, and CPUs, and Compilers! Oh My!"
    url: https://www.youtube.com/watch?v=nXaxk27zwlk
  - title: Google Benchmark
    url: https://github.com/google/benchmark
---
Is your `Vector` as fast as `std::vector`? Measuring that properly is
a skill of its own, because the easy ways get it wrong:

- **Debug builds lie.** Everything so far was built with `-O0` and
  sanitizers, which can be 10× slower and slow down different code by
  different amounts. This lesson's `optimize: true` builds the program
  with `-O2` and no sanitizers. (The tests keep them.)
- **The optimiser deletes unused work.** If a benchmark's result is never
  used, `-O2` may remove the loop entirely and report 0 ns.
  `do_not_optimize(x)` in `bench.hpp` tells the compiler the value is
  needed, without costing anything.
- **One measurement is noise.** The OS scheduler, CPU frequency changes
  and other programs all get in the way. Take several samples and report
  the **median**, which ignores the odd slow outlier where the mean
  wouldn't.
- **The clock has a resolution.** Timing a 2 ns operation once is
  meaningless. Time a batch of calls and divide.

```cpp
auto start = std::chrono::steady_clock::now();
for (int i = 0; i < reps; ++i) f();
auto ns = std::chrono::duration<double, std::nano>(
    std::chrono::steady_clock::now() - start).count();
```

`steady_clock` never jumps backwards when the system time is adjusted,
which `system_clock` can.

`main.cpp` runs a few races: your containers against the standard
library's, plus one that shows why linked lists are slow. Your job is
the harness that times them.

## Assignment

- [ ] `median` returns the middle value, or the mean of the two middle values
- [ ] `ns_per_call` runs `f` in `samples` batches of `reps` calls, and returns the median time per call in nanoseconds
- [ ] Run it and read the table: where does your code win, and where does it lose?
