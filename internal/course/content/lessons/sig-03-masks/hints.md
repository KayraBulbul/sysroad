```cpp
sigset_t set;
sigemptyset(&set);
for (int sig : sigs) {
    sigaddset(&set, sig);
}
sigprocmask(SIG_BLOCK, &set, &old_);
```

`SIG_BLOCK` adds to the mask instead of replacing it, which is what
makes nesting work. The destructor uses `SIG_SETMASK` with `old_`.
---
`sigsuspend` takes the mask to use *while* sleeping: the old mask
(which might block other things the caller cares about) without the
signals you're waiting for.

```cpp
sigset_t during = block.previous();
for (int sig : sigs) {
    sigdelset(&during, sig);
}
noted = 0;
while (noted == 0) {
    sigsuspend(&during);
}
```
---
Order matters at the end. Put the old actions back first, while the
signals are still blocked, then the mask (end the `BlockSignals`
scope). The other way round, a second signal that arrives in between
would run `note` and vanish, when the caller's own handler (or default
action) should have had it.
