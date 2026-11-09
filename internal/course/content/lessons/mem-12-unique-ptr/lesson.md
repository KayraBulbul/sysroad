---
title: "std::unique_ptr: one owner"
minutes: 25
files: [tree.cpp]
run:
  stdin: "src/main.cpp\nsrc/util/str.cpp\nREADME.md\nsrc/util/str.hpp\ndocs/intro.md\n"
deeper:
  - title: std::unique_ptr (cppreference)
    url: https://en.cppreference.com/w/cpp/memory/unique_ptr
  - title: "GotW #89: Smart Pointers (Herb Sutter)"
    url: https://herbsutter.com/2013/05/29/gotw-89-solution-smart-pointers/
---
`std::unique_ptr<T>` is a pointer that **owns** what it points at. When
the `unique_ptr` is destroyed, it deletes the object. It can't be
copied, because two owners would delete twice (the `Fd` lesson's
problem again). It can be **moved**, which hands ownership over and
leaves the source empty:

```cpp
auto e = std::make_unique<Entry>();      // new Entry, owned by e
Entry* raw = e.get();                    // look, don't own
std::unique_ptr<Entry> f = std::move(e); // f owns it now; e is null
// leaving the scope: f deletes the Entry, e has nothing to delete
```

`std::move` doesn't move anything by itself. It marks `e` as "you may
take from this", and `unique_ptr`'s move constructor does the taking.
Chapter 3 covers how.

You'll build a tree of names, the way `tree` draws a directory, from a
list of paths:

```
$ printf 'src/main.cpp\nsrc/util/str.cpp\nREADME.md\n' | ./tree
.
├── README.md
└── src
    ├── main.cpp
    └── util
        └── str.cpp

3 directories, 3 files
```

Each `Entry` holds its children as `std::vector<std::unique_ptr<Entry>>`.
Why not a plain `std::vector<Entry>`? Inserting into a vector can move
its elements to a new buffer, so any `Entry*` you were holding would
dangle. With `unique_ptr`, the vector only moves the *pointers*. Each
`Entry` stays where it was allocated, and its address never changes.
And when the root is destroyed, every `unique_ptr` deletes its child in
turn, so the whole tree is freed with no `delete` written anywhere.

## Assignment

- [ ] `child` finds a child by name, or inserts a new one in sorted position
- [ ] `add_path` adds each part of a `/`-separated path, skipping empty parts and `.`
- [ ] `detach` removes a child and returns ownership of it
- [ ] `print_tree` draws the tree with `├── `, `└── `, `│   ` and a summary line
