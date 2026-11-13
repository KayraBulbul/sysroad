```cpp
static int next_number(const Shell& sh) {
    int n = 0;
    for (const Job& j : sh.jobs) {
        n = std::max(n, j.number);
    }
    return n + 1;
}
```
---
`Job` keeps `ended` and `statuses` side by side with `pids`. In
`notify_jobs`, check each process that hasn't ended:

```cpp
int st = 0;
if (!job.ended[i] && waitpid(job.pids[i], &st, WNOHANG) == job.pids[i]) {
    job.ended[i] = true;
    job.statuses[i] = decode(st);
}
```

`job.done()` says whether they all have. Erase from a vector while
looping with `it = sh.jobs.erase(it)`.
---
The report's middle word comes from the last command's `Status`:
`s.ok()` is `Done`, an exit is `"Exit " + std::to_string(s.code)`, and
a signal is `signal_name(s.code)` from `proc.hpp`.
