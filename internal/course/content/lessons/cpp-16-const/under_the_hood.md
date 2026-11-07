**const is checked, then forgotten.** It doesn't change the machine code.
The compiler checks it and moves on. That's why `const_cast` can remove
it, and why doing that on an object that really is const (say, a string
literal) is undefined behaviour: it might live in read-only memory, and
writing to it segfaults.

**Where const things live.** A `constexpr` table or a `static const`
global goes into the `.rodata` section, which is mapped read-only. Phase 5's
mini readelf shows you the sections. `const` locals are usually just
registers.

**`mutable` is the escape hatch.** A cache inside a const member function
(for example, remembering an expensive result) can be marked `mutable`.
`std::mutex` members are often `mutable` too, so const readers can still
lock.

**East const.** `const int& x` and `int const& x` mean the same thing.
The rule that always works is that const applies to what's on its left,
unless there's nothing there. So `const char* p` is a pointer to const
chars, `char* const p` is a const pointer, and `const char* const p` is
both.
