---
title: Branch prediction
minutes: 35
files: [branches.cpp]
program: branches
optimize: true
run: {}
deeper:
  - title: "Why is processing a sorted array faster than an unsorted one? (Stack Overflow)"
    url: https://stackoverflow.com/questions/11227809
  - title: "CS:APP 3.6.6 and 5.11: conditional moves, and branch prediction penalties"
    url: https://csapp.cs.cmu.edu/
  - title: "Branchless binary search (Paul Khuong)"
    url: https://pvk.ca/Blog/2012/07/30/binary-search-is-a-pathological-case-for-caches/
---
A core works on dozens of instructions at once, so when it reaches a
branch it can't wait to find out which way it goes. It **predicts**,
keeps going down that path, and throws the work away if it guessed
wrong. A wrong guess (a **misprediction**) costs 15-20 cycles.

Predictors are very good at patterns: a loop branch that's taken 999
times then not, or a branch that alternates. They're helpless against
randomness. Here's the branch in the plain `filter_above`, which is in
`main.cpp` along with the plain binary search:

```cpp
if (in[i] > limit) {
    out[count++] = in[i];
}
```

On sorted input it goes the same way for half a million elements, then
flips once: nearly free. On shuffled input it's a coin toss, wrong
half the time, which costs more than everything else in the loop put
together. Same code, same data, 10× slower.

The cure is to remove the branch, and turn the decision into
arithmetic. Always write the element, and only *advance* when it was
one to keep:

```cpp
out[count] = in[i];
count += in[i] > limit;   // a bool is 0 or 1
```

There's still a comparison, but nothing depends on predicting it. The
cost is a store for every element, which is cheap.

Binary search has the same problem in its loop: whether to go left or
right is unpredictable by design. Rewrite it so the loop runs a fixed
number of steps, each choosing the next window with a conditional
expression instead of `if`/`else`:

```cpp
const int* base = a;
while (n > 1) {
    std::size_t half = n / 2;
    base = (base[half - 1] < x) ? base + half : base;
    n -= half;
}
```

The compiler can turn a `?:` like that into a **conditional move**
(`cmov` on x86, `csel` on arm64): both values are ready, and the
instruction picks one with no guessing. But the compiler is allowed to
turn it back into a branch, and clang does for this loop on x86-64.
Tell it the condition is a coin toss with clang's
`__builtin_unpredictable(cond)`, and it keeps the conditional move.

`perf stat -e branches,branch-misses ./branches` shows the
mispredictions directly. On the Mac, Instruments' CPU Counters
template does.

## Assignment

- [ ] `filter_above` has no `if`: it writes every element and advances by the comparison
- [ ] `lower_bound_index` halves a window with `?:` and `__builtin_unpredictable`, and handles the last element
- [ ] Run it. Why does your filter lose a little on sorted data?
