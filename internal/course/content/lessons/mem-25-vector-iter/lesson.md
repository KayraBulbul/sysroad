---
title: Vector iterators and invalidation
minutes: 25
files: [vector.hpp]
run: {}
deeper:
  - title: Iterator invalidation rules (cppreference)
    url: https://en.cppreference.com/w/cpp/container#Iterator_invalidation
  - title: Erase–remove idiom (Wikipedia)
    url: https://en.wikipedia.org/wiki/Erase%E2%80%93remove_idiom
---
An **iterator** is anything that can point at an element, step to the
next one and be compared. For a vector, a plain pointer already does all
of that, so `Vector<T>::iterator` is just `T*`. Give `Vector` a `begin()`
and an `end()`, and range-for, `std::sort`, `std::find` and the rest of
`<algorithm>` all work on it.

```cpp
for (const auto& w : words) ...                     // begin() to end()
std::sort(v.begin(), v.end());
auto it = std::find(v.begin(), v.end(), 42);        // end() if not found
```

The catch is **invalidation**. An iterator is a pointer into the
buffer, so anything that moves elements breaks it:

- growing (`push_back`, `insert`, `reserve`) can move the whole buffer:
  every iterator, pointer and reference is now dangling
- `insert` and `erase` shift the elements after the position, so
  iterators there point at different elements than before

That's why `insert` and `erase` *return* an iterator: the valid one to
carry on with. The classic bug is erasing inside a loop and then using
the old iterator. The fix is `it = v.erase(it);`.

The **erase–remove idiom** removes many elements in one pass.
`std::remove_if` moves the elements you keep to the front and returns
where the leftovers start, then one `erase` chops them off.

## Assignment

- [ ] `begin` and `end`, const and not
- [ ] `insert` puts a value before `pos`, even when the vector has to grow
- [ ] Both `erase`s shift the rest down, destroy what's left over, and return the right iterator
- [ ] `erase_if` removes matching elements with `std::remove_if` and your `erase`
