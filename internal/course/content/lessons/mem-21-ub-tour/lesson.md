---
title: A tour of undefined behaviour
minutes: 20
files: [ub.cpp]
run: {}
deeper:
  - title: "What Every C Programmer Should Know About Undefined Behavior (LLVM blog)"
    url: https://blog.llvm.org/2011/05/what-every-c-programmer-should-know.html
  - title: UndefinedBehaviorSanitizer (clang docs)
    url: https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
---
**Undefined behaviour** (UB) is code the standard puts no requirements
on at all. It isn't "crashes" or "gives a random number". The compiler
is allowed to *assume it never happens*, and it optimises on that
assumption:

```cpp
bool will_wrap(int x) {
    return x + 1 < x;     // signed overflow is UB, so this "can't" be true
}                         // clang -O2 compiles it to: return false
```

That's why UB bugs are so confusing. The program can behave differently
at `-O0` and `-O2`, or after an unrelated change, and the bug can show
up far from the line that caused it.

You've met some already: out-of-bounds access, use after free and
dangling pointers. `ub.cpp` has five more, all common in real code:

- **signed overflow**: `int` arithmetic that leaves the range of `int`
- **an oversized shift**: shifting a 32-bit value by 32 or more
- **a misaligned read**: reading a `uint32_t` from an address that isn't
  a multiple of 4
- **reading one past the end** of a vector
- **negating `INT_MIN`**: -2147483648 has no positive twin in an `int`

All five get caught at run time: by UBSan (`runtime error: ...`), by
ASan, or, for the vector, by the standard library's own bounds check
(`Assertion '__n < this->size()' failed`), which some builds switch on
in debug mode. Run, read the report, fix it, and repeat. Each comment in `ub.hpp` says what
the function promises, edge cases included.

## Assignment

- [ ] `midpoint` can't overflow
- [ ] `rotl` handles any shift amount, including 0 and 32
- [ ] `read_u32` reads from any address
- [ ] `has_adjacent_equal` stays inside the vector
- [ ] `magnitude` works for every `int`
