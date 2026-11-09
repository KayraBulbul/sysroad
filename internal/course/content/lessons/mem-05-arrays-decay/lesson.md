---
title: "Arrays, pointers and decay"
minutes: 15
files: [arrays.cpp]
run: {}
deeper:
  - title: Array-to-pointer conversion (cppreference)
    url: https://en.cppreference.com/w/cpp/language/array#Array-to-pointer_decay
  - title: std::array (cppreference)
    url: https://en.cppreference.com/w/cpp/container/array
---
A C array is its elements, back to back, with nothing else: no length,
no header. `int scores[5]` is 20 bytes, and inside the function that
declares it `sizeof scores` is 20.

Pass it to a function, though, and it **decays** to a pointer to its
first element. The length is gone. Even writing the parameter as an
array doesn't help, because the compiler quietly rewrites it:

```cpp
void show(const int values[5]) {   // really: const int* values
    sizeof values;                 // 8, the size of a pointer
}
```

So C-style code passes a pointer and a length, or a pointer to the
first element and one **past the last**, which is how the standard
library does it (`v.begin()`, `v.end()`). One past the end is a valid
pointer to compare against, just never to dereference:

```cpp
for (const int* p = first; p != last; ++p)
    total += *p;
```

`std::array<int, 5>` is the fix when the length is fixed. It's the same
20 bytes, with no overhead, but it's a real type: it knows its
`size()`, compares with `==`, and is copied when passed by value
instead of decaying.

## Assignment

- [ ] `sum` adds up the ints from `first` up to (not including) `last`
- [ ] `reverse` reverses them in place by swapping from both ends
- [ ] `doubled` returns a copy of a `std::array` with every element doubled
