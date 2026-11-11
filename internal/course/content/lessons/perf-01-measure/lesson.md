---
title: Measuring time properly
minutes: 30
files: [bench.cpp]
optimize: true
run: {}
deeper:
  - title: "CS:APP chapter 5 and 6 (Bryant & O'Hallaron): the book this phase follows"
    url: https://csapp.cs.cmu.edu/
  - title: "Google Benchmark: how it picks the number of iterations"
    url: https://github.com/google/benchmark/blob/main/docs/user_guide.md
  - title: "Producing wrong data without doing anything obviously wrong! (Mytkowicz et al.)"
    url: https://dl.acm.org/doi/10.1145/1508284.1508275
---
This phase is about making code fast, which starts with knowing how
fast it is. Lesson 2.4.7 gave you `ns_per_call(f, reps, samples)`. It
works, but you had to pick `reps` by hand, and a bad pick ruins the
result:

- **Too few reps** and the clock can't see the work. `steady_clock`
  ticks every 15-20 ns on Linux and every 41.7 ns on an Apple Silicon
  Mac. Timing one 5 ns call measures the clock, not the call.
- **Too many** and a fast machine spends seconds on one number, while
  a slow one never finishes.

The fix is to let the harness pick. Start with one call, double until
one batch takes long enough to be measured well (a millisecond is
plenty), and use that many reps for every sample. Google Benchmark
does the same.

Two more traps:

- **The first run is different.** It pays for page faults on fresh
  memory, cold caches, branch predictors that haven't learnt the code
  yet, and a CPU that is still ramping up its clock speed. Throw it
  away. Calibrating already runs `f` plenty of times, so the timed
  samples come after a warm-up for free, as long as you don't count
  the calibration runs.
- **One number hides how noisy it was.** Report the min, median and
  max. The **min** is the run with the least interference, which
  makes it the best estimate of what the code itself costs. The
  **median** is what a typical run costs. The **spread**, max minus
  min over the median, tells you whether to trust either: a spread
  of 2% is a clean measurement, and one of 50% means something else
  was going on.

The functions take the clock as a parameter. Your program uses the
real one, `steady_ns()`, which is already written. The tests pass a
fake clock that moves forward a fixed amount each time `f` is called,
so they can check your logic exactly without depending on how fast
this machine is.

```cpp
double t = 0;
Clock fake = [&] { return t; };
auto f = [&] { t += 100; };     // every call "takes" 100 ns
calibrate(f, 1000, fake);       // 16: the first batch of at least 1000 ns
```

## Assignment

- [ ] `summarize` sorts the per-call times and fills in `min`, `median`, `max` and `samples`
- [ ] `calibrate` times batches of 1, 2, 4, … calls and returns the first `reps` whose batch takes at least `target_ns`, or `max_reps`
- [ ] `measure` calibrates, then times `samples` batches of that many calls and summarizes the time per call, with `reps` filled in
- [ ] Run it, and compare the first and second `memset` of the same 256 MB, and the list rows at the bottom
