**It costs nothing.** A `std::unique_ptr<T>` is exactly one pointer in
size, and `*p` or `p->x` compile to the same instruction as a raw
pointer. The only extra code is the `delete` in its destructor, which
you'd have written anyway. That's why "use `unique_ptr` for owning
pointers" is close to a rule in modern C++.

**Owning vs observing.** The convention: a `std::unique_ptr` *owns*, and
a raw `T*` or `T&` only *observes*. A function that takes `Entry*`
promises not to delete it. A function that takes
`std::unique_ptr<Entry>` by value takes ownership, and the caller has to
`std::move` it in, so the transfer is visible at the call site.

**`release` and `reset`.** `p.release()` gives up ownership and returns
the raw pointer, so you must delete it yourself. `p.reset(q)` deletes
the old object and owns `q`. These are the same two operations as your
`Fd` class from phase 1, because `Fd` was a unique owner of a file
descriptor.

**Custom deleters.** `std::unique_ptr<FILE, decltype(&fclose)> f(fopen(...), &fclose);`
owns a C `FILE*` and calls `fclose` instead of `delete`. Any C API with
a "create" and a "destroy" function can be wrapped this way.

**Recursive destruction.** Destroying the root destroys its children,
which destroy theirs: the destructor recurses as deep as the tree. A
directory tree is shallow, so that's fine. A linked list of a million
`unique_ptr` nodes is a million-deep "tree", and its destructor
overflows the stack. Lesson 2.2.6 meets that problem.
