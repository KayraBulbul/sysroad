A job, as one shape that works:

```cpp
struct Job {
    int number = 0;
    std::vector<pid_t> pids;      // one per command
    std::vector<bool> ended;      // side by side with pids...
    std::vector<int> statuses;    // ...and what waitpid said
    std::string text;
};
```

and a `std::vector<Job> jobs;` in your `Shell`. The text is easy to
build while parsing: append each token's text, with a space between.
---
The next number:

```cpp
int n = 0;
for (const Job& j : sh.jobs) n = std::max(n, j.number);
return n + 1;
```

In the background child that runs the first command, before its other
redirections: `int null = open("/dev/null", O_RDONLY); dup2(null, 0);
close(null);`.
---
Noticing, for each process that hasn't ended:

```cpp
int st = 0;
if (!job.ended[i] && waitpid(job.pids[i], &st, WNOHANG) == job.pids[i]) {
    job.ended[i] = true;
    job.statuses[i] = st;
}
```

Then report and erase the jobs that are all done, with
`it = sh.jobs.erase(it)` in the loop. Call it at the top of `main`'s
loop, and at the start of `jobs`.
---
The notice's middle word comes from the last command's status:
`WIFSIGNALED(st)` gives the signal's name (a `switch` over the common
ones, `SIGTERM` and so on), an exit status of 0 is `Done`, and anything
else is `"Exit " + std::to_string(WEXITSTATUS(st))`.
