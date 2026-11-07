---
title: Sanitizers catch five bugs
minutes: 15
xp: 10
files: [bugs.cpp]
deeper:
  - title: AddressSanitizer (clang docs)
    url: https://clang.llvm.org/docs/AddressSanitizer.html
  - title: UndefinedBehaviorSanitizer (clang docs)
    url: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
---
In C++, many bugs don't crash. Reading one element past the end of a
vector returns whatever happens to be in memory next. The program carries
on, and the wrong number shows up three functions later, or only on the
other machine. **Sanitizers** make those bugs fail *at the line that
causes them*, with a report.

Every lesson build already uses them (see Check your toolchain):

- **AddressSanitizer** (ASan) catches out-of-bounds reads and writes, use
  after free, and use after return.
- **UndefinedBehaviorSanitizer** (UBSan) catches signed integer overflow,
  bad shifts, null dereferences, and misaligned pointers.

`bugs.cpp` has five small functions with one classic bug each. Press Run,
and the first test dies with a report like this one:

```text
ERROR: AddressSanitizer: heap-buffer-overflow on address 0x7b67f97e0080
READ of size 4 at 0x7b67f97e0080 thread T0
    #0 sum_first(std::vector<int> const&, unsigned long) bugs.cpp:10:16
    #1 DOCTEST_ANON_FUNC_2() tests/visible_test.cpp:13:11

0x7b67f97e0080 is located 0 bytes after 16-byte region [...)
allocated by thread T0 here:
    #0 DOCTEST_ANON_FUNC_2() tests/visible_test.cpp:12:22
```

Read it from the top. The first line says *what* went wrong
(`heap-buffer-overflow`), the stack says *where* (`bugs.cpp:10`, called from the test on line 13),
and the next part says *how far* (0 bytes past the end of a 16-byte
buffer, which holds 4 ints) and who allocated it. That's an off-by-one.
sysroad trims the report to your code. The full one also lists the
standard library and runtime frames, and a map of "shadow memory".

Fix the bugs one at a time. Each fix uncovers the next report.

## Assignment

- [ ] 1: `sum_first` reads past the end
- [ ] 2: `first_then_grow` uses memory after it's freed
- [ ] 3: `average` overflows a signed int
- [ ] 4: `welcome` returns a view of a dead string
- [ ] 5: `count_digit` writes outside a local array
