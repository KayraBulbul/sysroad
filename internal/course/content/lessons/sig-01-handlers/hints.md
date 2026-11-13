```cpp
struct sigaction action{};
action.sa_handler = handler;
action.sa_flags = flags;
sigemptyset(&action.sa_mask);
struct sigaction old{};
if (sigaction(sig, &action, &old) == -1) {
    throw std::system_error(errno, std::generic_category(), "sigaction");
}
return old;
```

`struct sigaction` needs the `struct` in front, because there's also a
function called `sigaction`.
---
`usleep(interval_ms * 1000)` returns early when a signal arrives. So
the order in the loop is: sleep, check the flag (stop if it's set),
then count and print the tick.
