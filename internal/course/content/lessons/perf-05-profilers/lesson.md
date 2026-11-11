---
title: "perf on Linux, Instruments on the Mac"
minutes: 40
files: [wordfreq.cpp]
program: wordfreq
optimize: true
run:
  args: ["--demo", "150000"]
deeper:
  - title: "Brendan Gregg's perf examples"
    url: https://www.brendangregg.com/perf.html
  - title: "Flame graphs"
    url: https://www.brendangregg.com/flamegraphs.html
  - title: "Apple: Analyzing CPU usage with the Time Profiler"
    url: https://developer.apple.com/documentation/xcode/analyzing-cpu-usage-in-your-app
---
`wordfreq` counts the words in a text and prints the ten most common.
It's correct, and on a real book it's painfully slow. Where's the time
going? Read the code and you'll find suspects: a string rebuilt for
every letter, a full sort when only ten results are needed, a function
call per character. Programmers who guess at this are wrong most of
the time. A **profiler** tells you.

A sampling profiler interrupts your program thousands of times a
second and records which function it was in, and which functions
called that one. Functions that show up in many samples are where the
time goes. It needs symbols (`-g`) and, to walk the stack cheaply,
frame pointers (`-fno-omit-frame-pointer`), on an optimised build.
Profiling a `-O0` build finds the slow parts of code you won't ship.

Build it in the lesson folder:

```sh
clang++ -std=c++20 -O2 -g -fno-omit-frame-pointer *.cpp -o wordfreq
```

**Linux: `perf`.**

```sh
perf record -g --user-callchains ./wordfreq --demo 150000
perf report
```

`perf report` lists functions with two columns. **Self** is time spent
in the function's own code. **Children** (or total) adds everything it
called. A function with a big total and a small self is just a caller;
keep expanding (`+` or Enter) until you find the one doing the work.
`--user-callchains` is there because Omarchy only lets normal users
profile their own code, not the kernel. Code that clang inlined still
shows up, marked `(inlined)`.

`perf stat ./wordfreq --demo 150000` counts instead of sampling:
cycles, instructions, cache misses and branch misses for the whole
run.

**macOS: Instruments, or `sample`.** With Xcode installed, the Time
Profiler records the same thing:

```sh
xcrun xctrace record --template 'Time Profiler' --launch -- ./wordfreq --demo 150000
open *.trace
```

In Instruments, open the call tree and tick **Invert Call Tree** and
**Hide System Libraries** to put the hottest functions on top. Without
Xcode, `sample` comes with macOS: start the program in one terminal
with a bigger input, and run `sample wordfreq 3` in another to get a
text call tree of 3 seconds.

Find the hotspot and fix it. Keep the functions' behaviour the same.
`count_words` can return its words in any order.

## Assignment

- [ ] Profile `wordfreq` and find which function takes the time
- [ ] Fix it, so counting 100,000 words takes well under a second
- [ ] Profile it again. What's on top now?
