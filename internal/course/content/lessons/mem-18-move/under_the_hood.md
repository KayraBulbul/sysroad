**Move-only types.** `Fd`, `std::unique_ptr`, `std::thread` and
`std::fstream` can be moved but not copied. They model a resource that
has exactly one owner at a time. Containers handle them fine. You just
`std::move` them in, and the compiler stops you if you forget.

**`std::exchange`.** A common way to write moves:
`Fd(Fd&& other) noexcept : fd_(std::exchange(other.fd_, -1)) {}`.
`std::exchange(x, v)` sets `x` to `v` and returns the old value, which
is "take it and leave this behind" in one call.

**Implicit moves on return.** `return fd;` where `fd` is a local (or a
by-value parameter) is treated as an rvalue first. That's why returning
a named move-only object works without `std::move`. C++20 and C++23
widened the cases where this happens automatically.

**Self-move-assignment.** `a = std::move(a)` almost never happens on
purpose, but it can sneak in through algorithms that swap or rotate
elements in place. The standard library only promises that it leaves
the object valid. For your own resource types, keeping the resource
intact is the friendly choice and costs one comparison.

**`dup` vs move.** If you really do need two descriptors for the same
open file, `::dup(fd)` makes a second one that the kernel tracks
separately. A copy constructor *could* call `dup`, which is what a
"copyable Fd" would look like. Most designs choose move-only, so a copy
can't happen by accident.
