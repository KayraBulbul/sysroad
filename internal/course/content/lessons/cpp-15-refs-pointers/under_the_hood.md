**The machine code is the same.** Compile `void inc(int& x) { ++x; }` and
`void inc(int* x) { ++*x; }` with `-O2` and compare: both load through
the address in `rdi`. References are a rule the compiler checks, not a
different kind of value. Try it on godbolt.org.

**Range-for copies unless you say otherwise.** `for (int x : v)` copies
each element, so `&x` would be the address of a local that changes every
iteration. `for (int& x : v)` binds to the element itself. For strings
and other big objects, `for (const auto& s : v)` avoids a copy per
element.

**Pointers into a vector are fragile.** `find_value` hands out `&v[i]`.
If the caller then does `v.push_back(...)` and the vector reallocates,
that pointer dangles, which is the same bug as the `string_view` quiz
question. Phase 2 builds a Vector so you can see exactly when it happens.

**A `T&` member makes a class awkward.** It can't be re-pointed, so the
class can't be assigned to. That's why containers of references
(`std::vector<int&>`) don't exist and you see `std::reference_wrapper` or
plain pointers instead.
