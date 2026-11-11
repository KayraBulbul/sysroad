---
title: The memory hierarchy
minutes: 35
files: [cache.cpp]
run: {}
deeper:
  - title: "CS:APP 6.2-6.4: locality and cache memories"
    url: https://csapp.cs.cmu.edu/
  - title: "What Every Programmer Should Know About Memory (Ulrich Drepper)"
    url: https://people.freebsd.org/~lstewart/articles/cpumemory.pdf
  - title: "Latency numbers every programmer should know"
    url: https://gist.github.com/jboner/2841832
---
An add takes a quarter of a nanosecond. Reading a number that isn't in
any cache takes around 80 ns, long enough for over 300 adds. CPUs hide
that gap with a stack of caches, each bigger and slower than the one
above it:

| | size (per core, roughly) | latency |
|---|---|---|
| registers | a few hundred bytes | 0 |
| L1 data cache | 32-128 KB | ~1 ns (4-5 cycles) |
| L2 | 1-16 MB | ~4 ns |
| L3 (shared) | 8-36 MB | ~10-20 ns |
| main memory | GBs | ~80-100 ns |

`lscpu` and `getconf -a | grep CACHE` show yours on Linux. On the Mac,
`sysctl -a | grep cache`.

**Caches move lines, not bytes.** Memory is split into 64-byte
**lines** (128 on Apple Silicon). Reading one byte loads its whole
line, so the next 63 bytes are free. That's why reading an array in
order is fast and jumping around it is slow.

**Where a line can go.** A cache is split into **sets**, each holding
a few lines, its **ways**. An address splits into three parts:

```text
address = | tag | set | offset |
              |     |      └─ which byte in the line: addr % line
              |     └──────── which set: (addr / line) % sets
              └────────────── tells apart lines in that set: addr / line / sets
```

A line can only go in its own set. When the set is full, one of its
lines has to go, usually the **least recently used** (LRU). With one
way it's *direct-mapped*. With one set holding every line it's *fully
associative*. Real L1s sit in between: 32 KB with 8 ways is 64 sets.

Every miss is one of three kinds:

- **compulsory**: the first touch of a line. Nothing helps but
  touching fewer lines.
- **capacity**: the data you're cycling through is bigger than the
  cache.
- **conflict**: the cache has room, but too many of your lines map to
  the same set. A power-of-two stride is the classic cause.

You'll build a simulator that counts hits and misses for any cache
shape. `main.cpp` runs some access patterns through a 32 KB L1. The
tests use small caches you can check by hand.

## Assignment

- [ ] `sets`, `set_of` and `tag_of` split an address the way the diagram does
- [ ] `access` returns whether the line is in its set, and records the hit or miss
- [ ] On a miss it adds the line, evicting the set's least recently used line if the set is full
- [ ] Run it. Why does the column walk miss every time, and why does padding each row fix it?
