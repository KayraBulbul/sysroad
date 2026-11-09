---
title: "Leaks, double frees and how ASan finds them"
minutes: 20
files: [bugs.cpp]
run: {}
deeper:
  - title: AddressSanitizer (clang docs)
    url: https://clang.llvm.org/docs/AddressSanitizer.html
  - title: LeakSanitizer (clang docs)
    url: https://clang.llvm.org/docs/LeakSanitizer.html
---
Manual `new` and `delete` go wrong in four classic ways, and `bugs.cpp`
has one of each:

- a **leak**: some path out of the function never calls `delete`
- a **use after free**: reading memory after handing it back
- a **double free**: deleting the same pointer twice
- a **mismatched delete**: `delete` on something made with `new[]`

Without sanitizers, the last three are undefined behaviour that often
seems to work, and leaks just quietly grow. ASan turns them into
reports. A heap report has up to three stacks, and they tell the whole
story:

```
ERROR: AddressSanitizer: heap-use-after-free on address 0x7bad2f3e0010
READ of size 2 at 0x7bad2f3e0010 thread T0
    #0 shout(std::string const&) bugs.cpp:38:26     ← where you used it
    #1 main main.cpp:11:53
freed by thread T0 here:
    #0 shout(std::string const&) bugs.cpp:37:5      ← where it was freed
previously allocated by thread T0 here:
    #0 shout(std::string const&) bugs.cpp:31:18     ← where it came from
```

Leaks show up when the program exits, through **LeakSanitizer** on
Linux: `Direct leak of 24 byte(s) in 1 object(s) allocated from: ...`.
macOS has no LeakSanitizer, so the tests here count allocations with
`alloc_count.hpp` from the last lesson. A test fails if anything
allocated during a call is still alive afterwards. On macOS, try
`leaks --atExit -- ./program` for the system's own leak finder.

Run first and read each report, fix that bug, and run again. One bug
also triggers a compiler warning. Look at the warnings before you run.

## Assignment

- [ ] Fix the leak in `sum_numbers`
- [ ] Fix the use after free in `shout`
- [ ] Fix the double free that `doubled_copy` causes
- [ ] Fix the mismatched delete in `count_unique`
