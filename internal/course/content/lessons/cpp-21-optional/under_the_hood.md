**An optional is the value plus a bool.** `sizeof(std::optional<int>)`
is 8: 4 for the int, 1 for "has value", plus padding. The value lives
inside the optional, so there's no heap allocation. That's what makes it
fine to return by value everywhere.

**`*opt` on an empty optional is undefined behaviour.** `opt.value()`
checks and throws `std::bad_optional_access` instead. Use `*` after
you've tested, and `value()` when "empty" would be a bug you want to
hear about.

**C++23 has `std::expected<T, E>`,** an optional that carries *why* it
failed: `std::expected<int, ParseError>`. It's the same idea as Go's `(v,
err)` and Rust's `Result`. This course compiles as C++20, so it uses
optional plus a message. You'll see `expected` in newer codebases.

**Why not `atoi` or `stoi`?** `atoi("12abc")` returns 12 and
`atoi("abc")` returns 0, so there's no way to tell an error from zero.
`std::stoi` throws on garbage but quietly accepts `"12abc"`, and it skips
leading spaces using the current locale. `from_chars` is the only one of
the three that's both exact and fast.

**Signed overflow is UB, unsigned wraps.** `n * mult` on `std::uint64_t`
can't crash. It wraps around modulo 2⁶⁴ and gives a silently wrong size.
The divide-first check is the standard way to test for that before
multiplying.
