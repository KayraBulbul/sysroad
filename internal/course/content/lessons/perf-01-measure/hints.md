`summarize` is lesson 2.4.7's `median` with three more fields:

```cpp
std::sort(xs.begin(), xs.end());
Stats s;
s.min = xs.front();
s.max = xs.back();
// … median as before
s.samples = static_cast<int>(xs.size());
return s;
```
---
Both `calibrate` and `measure` time a batch, so write that once:

```cpp
static double time_batch(const std::function<void()>& f, long reps, const Clock& now) {
    double start = now();
    for (long i = 0; i < reps; ++i) {
        f();
    }
    return now() - start;
}
```

Then `calibrate` is a loop that doubles `reps` while it's below
`max_reps` and the batch was too short. Check `max_reps` first, so you
don't run a batch you won't use.
---
`measure` calls `calibrate` and ignores how long that took. Then each
sample is `time_batch(f, reps, now) / reps`. Collect them in a vector,
`summarize` it, and set `reps` on the result before returning it.
