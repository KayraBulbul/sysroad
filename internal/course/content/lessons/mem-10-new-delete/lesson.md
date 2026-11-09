---
title: "new, delete and what they cost"
minutes: 15
files: [heap.cpp]
run: {}
deeper:
  - title: new expression (cppreference)
    url: https://en.cppreference.com/w/cpp/language/new
  - title: "What every programmer should know about memory (Drepper), section 2"
    url: https://people.freebsd.org/~lstewart/articles/cpumemory.pdf
---
A local lives in its function's frame and dies with it. When something
must outlive the function, or its size is only known at run time, it
goes on the **heap**:

```cpp
Point* p = new Point{1.0, 2.0};   // allocate, then construct
delete p;                         // destroy, then free

int* a = new int[n];              // n ints
delete[] a;                       // [] for arrays: always paired
```

`new` does two things. It asks the allocator for `sizeof(Point)` bytes
(`operator new`, which usually calls `malloc`), then runs the
constructor in them. `delete` runs the destructor, then hands the bytes
back. `new[]` must be paired with `delete[]` and `new` with `delete`.
Mixing them, or mixing in `malloc`/`free`, is undefined behaviour.

What it costs:

- **time**: a call into the allocator, which has to find a free block,
  usually tens of nanoseconds, and sometimes a system call to get more
  memory from the kernel. A local costs nothing.
- **space**: the allocator keeps 8–16 bytes of bookkeeping per block,
  so a `new int` uses far more than 4 bytes.
- **responsibility**: someone has to `delete` it exactly once. The next
  lesson is what happens when nobody does.

`alloc_count.cpp` replaces the global `operator new` and
`operator delete` with versions that count every call. `main.cpp` uses
it to show where your programs allocate, including some places you
might not expect.

## Assignment

- [ ] `make_filled` returns a new array of `n` copies of `value`
- [ ] `grow` copies an array into a bigger (or smaller) new one, zero-fills the rest and frees the old one
- [ ] `vector_allocations` counts how many allocations pushing `n` ints into a vector makes, with and without `reserve`
