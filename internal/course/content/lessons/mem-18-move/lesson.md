---
title: "Move constructors: Fd learns to move"
minutes: 20
files: [move.cpp]
run:
  args: ["a.txt", "missing.txt", "b.txt"]
  files:
    a.txt: "first file\n"
    b.txt: "second file\n"
deeper:
  - title: Move constructors (cppreference)
    url: https://en.cppreference.com/w/cpp/language/move_constructor
  - title: Move assignment (cppreference)
    url: https://en.cppreference.com/w/cpp/language/move_assignment
---
Your `Fd` from lesson 1.3.4 can't be copied, which is right: two owners
would close twice. But it can't be *moved* either, so you can't keep
one in a `std::vector`, return a named one from a function, or hand it
to another owner.

A move is the safe way to transfer ownership. The new object takes the
descriptor, and the old one is left holding `-1`, so its destructor has
nothing to close:

```cpp
Fd::Fd(Fd&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}
```

Move assignment also has to deal with what the left side already owns:
close it first, then take over. And `a = std::move(a)` mustn't close
the descriptor it's about to keep.

Both are marked `noexcept`. Swapping an int can't fail, and the promise
matters. Lesson 2.3.5 shows what `std::vector` does without it.

(How did `open_read` return an `Fd` before this? `return Fd(...);`
builds the result straight into the caller's variable: guaranteed copy
elision since C++17, with no copy or move involved. Returning a *named*
local, as `open_first` does, needs a real move constructor.)

`fd.cpp` has the rest of `Fd` from 1.3.4. `main.cpp` opens every file
it's given into a `std::vector<Fd>`, then prints them all:

```
$ ./move a.txt missing.txt b.txt
move: missing.txt: can't open
opened 2 of 3
first file
second file
```

## Assignment

- [ ] The move constructor takes the descriptor and leaves `other` empty
- [ ] Move assignment closes what it had, takes `other`'s, and survives `a = std::move(a)`
- [ ] `open_all` opens each path into a vector, listing the ones that fail
- [ ] `open_first` returns the first path that opens
