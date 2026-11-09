**`std::move` is a cast.** Its whole implementation is
`static_cast<std::remove_reference_t<T>&&>(t)`. No code runs. The
name is famously misleading; `std::rvalue` would have been more
honest.

**What a string move costs.** Three pointer-sized copies and zeroing the
source: no allocation, no character copying. For short strings stored
inline (the SSO from lesson 2.2.2) the characters *are* copied, but
there are at most 15 or 22 of them.

**Copy elision.** `return local;` usually doesn't even move. The
compiler builds the result directly in the caller's variable (*named
return value optimisation*). Since C++17, `return Tracked("x");` is
*guaranteed* not to copy or move at all. `return std::move(local);`
turns off NRVO, and clang warns about it with `-Wpessimizing-move`.

**Moved-from objects.** The standard library promises moved-from
objects are "valid but unspecified". A moved-from `std::string` is
empty in practice, and so is a `std::vector`. Your own types should
leave a moved-from object at least destructible and assignable. Lesson
2.3.3 does that for `Fd`.

**Forwarding references.** In a template, `T&&` means something
different: it binds to *anything* and remembers whether it was an
lvalue or rvalue. `std::forward<T>(x)` passes that on. That's how
`emplace_back(args...)` hands its arguments to a constructor untouched.
You'll use it when you build `Vector<T>`.
