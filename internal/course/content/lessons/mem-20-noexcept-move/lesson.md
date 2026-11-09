---
title: noexcept moves and vector growth
minutes: 15
files: [records.hpp]
run: {}
deeper:
  - title: std::move_if_noexcept (cppreference)
    url: https://en.cppreference.com/w/cpp/utility/move_if_noexcept
  - title: noexcept specifier (cppreference)
    url: https://en.cppreference.com/w/cpp/language/noexcept_spec
---
When a `std::vector` runs out of room, it allocates a bigger buffer and
transfers every element across. It *could* move them, but think about
what happens if a move throws halfway through. Half the elements are in
the new buffer, the old ones they came from have been gutted, and
there's no way back. `push_back` promises that if it throws, the vector
is unchanged (the *strong guarantee*).

So the vector only moves elements whose move constructor is
**`noexcept`**, a promise that it can't throw. Otherwise it copies
them, because a failed copy leaves the originals intact. That decision
is `std::move_if_noexcept`, and it happens silently.

```cpp
Record(Record&& o) noexcept            // vector growth moves
    : name(std::move(o.name)), scores(std::move(o.scores)) {}
```

`records.hpp` has two classes that look fine and quietly copy:

- `Record` writes its moves out by hand and forgets `noexcept`.
- `Line` declares a copy constructor (to count copies while
  debugging), and that switches its moves off entirely. Even
  `std::move(line)` copies.

`main.cpp` counts the allocations. Make both moves cheap and `noexcept`.

## Assignment

- [ ] Growing a `std::vector<Record>` moves the records instead of copying them
- [ ] Moving a `Line` doesn't allocate
- [ ] Both types are `noexcept` movable
