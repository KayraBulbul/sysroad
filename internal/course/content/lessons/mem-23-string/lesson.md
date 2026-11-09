---
title: "String, with the small-string optimisation"
minutes: 60
files: [string.cpp]
run: {}
deeper:
  - title: "libc++'s std::string (Joel Laity)"
    url: https://joellaity.com/2020/01/31/string.html
  - title: std::basic_string (cppreference)
    url: https://en.cppreference.com/w/cpp/string/basic_string
---
Time to build the containers you've been using. First, `String`: a
growable, `'\0'`-terminated run of chars that owns its memory, with the
trick every real `std::string` uses. **Short strings live inside the
object** and never touch the heap.

```cpp
class String {
    char* heap_ = nullptr;        // nullptr while the text is inline
    std::size_t size_ = 0;
    std::size_t cap_ = 15;        // chars that fit without growing
    char buf_[16] = {};           // the inline text, plus its '\0'
};
```

`data()` is `heap_ ? heap_ : buf_`. Most strings in real programs are
short (names, keys, words), so most strings never allocate.

You've written every piece of this already:

- **growing**: allocate a bigger buffer, copy, free the old one (2.2.2)
- **deep copies**, and assignment that survives `s = s` (2.3.1)
- **moves** that steal the heap buffer and leave the source empty (2.3.3)
- **all five special members** (2.3.4), and `noexcept` moves (2.3.5)

One new trap: **aliasing**. `s.append(s)` passes a view of `s` into
`s`. If `append` grows by freeing the old buffer before copying from
the view, it reads freed memory. Build the new buffer completely, then
free the old one.

Growth should be geometric (double the capacity at least), so a
thousand `push_back`s cost a handful of allocations, not a thousand.
`string.hpp` documents every member. `main.cpp` shows sizes,
capacities and allocation counts as you go.

## Assignment

- [ ] Constructors and the destructor; strings of 15 chars or fewer never allocate
- [ ] `reserve`, `push_back`, `append` and `clear`, growing geometrically
- [ ] Copy constructor and copy assignment (deep, and safe for `s = s`)
- [ ] Move constructor and move assignment: no allocation, and the source is left empty
- [ ] `s.append(s)` works, even when it has to grow
