Write a helper that visits one target and returns its time once it's up
to date, and remember the answer in a `std::map<std::string, long>`:

- not in `rules`: it's a source file. Its time is its mtime, or throw if it has none.
- in `rules`: visit every dependency and keep the newest time. If the
  target is missing or older than that, add its command.

A target you just rebuilt counts as brand new, so give it a time larger
than anything on disk, like `std::numeric_limits<long>::max()`.
---
The remembered times give you "at most once" for free: the second visit
returns the stored time without adding the command again.

For cycles, keep a `std::vector<std::string>` of the targets you're in
the middle of visiting. Push before visiting the dependencies, pop
after. If a target is already on that list, you've looped: the message
is the list from that target's position, plus the target again.
---
A struct keeps the shared state out of every function signature:

```cpp
struct Planner {
    const Rules& rules;
    const Mtimes& mtimes;
    std::vector<std::string> commands;
    std::map<std::string, long> done;
    std::vector<std::string> path;
    long visit(const std::string& name);
};
```
