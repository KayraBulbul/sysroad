---
title: "Vector<T>: growth, placement new, destruction"
minutes: 60
files: [vector.hpp]
run: {}
deeper:
  - title: Placement new (cppreference)
    url: https://en.cppreference.com/w/cpp/language/new#Placement_new
  - title: std::destroy_at (cppreference)
    url: https://en.cppreference.com/w/cpp/memory/destroy_at
---
`String` stored `char`s, which need no constructing or destroying. A
`Vector<T>` holds any `T`, and that changes things. Its buffer has
**room** for `capacity()` elements, but only the first `size()` of them
exist as objects. So memory and objects have to be handled separately:

```cpp
T* buf = static_cast<T*>(::operator new(n * sizeof(T)));  // raw bytes, no T yet
new (buf + i) T(args...);    // placement new: construct a T in that spot
buf[i].~T();                 // destroy it, leaving raw bytes again
::operator delete(buf);      // give the bytes back
```

`new T[n]` would construct `n` objects up front, which is wrong for a
vector that's only partly full, and impossible for a `T` with no default
constructor.

Growing means allocating a bigger raw buffer, **moving** each element
across with placement new (`std::move_if_noexcept`, as in 2.3.5),
destroying the originals and freeing the old buffer.

Two details trip everyone up:

- `emplace_back(args...)` forwards its arguments to `T`'s constructor
  with `std::forward<Args>(args)...`. It's the template "forwarding
  reference" from 2.3.2's notes.
- `v.push_back(v[0])` passes a reference *into* the vector. If you free
  the old buffer before constructing the new element, that reference
  dangles. Construct the new element in the new buffer first, then move
  the old ones over.

Everything goes in `vector.hpp`, because a template's code has to be
visible wherever it's used. The tests use types that count their
constructions and destructions, so every object you create must be
destroyed exactly once.

## Assignment

- [ ] `reserve` grows into a new raw buffer, moving the elements, and never shrinks
- [ ] `push_back` (copy and move) and `emplace_back`, doubling the capacity when full
- [ ] `pop_back`, `clear` and `resize` construct and destroy exactly the right elements
- [ ] Copy and move constructors and assignments, and the destructor
- [ ] `v.push_back(v[0])` works when it has to grow
