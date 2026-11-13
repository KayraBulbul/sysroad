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
Order matters at the end. Put the mask back first (end the
`BlockSignals` scope), then the old actions. The other way round, a
second signal that arrives in between would find its default action
waiting, and for `SIGUSR1` that's death.
