**Measuring it.** A `std::vector<Record>` growing to 1000 elements
reallocates about 10 times and transfers 1023 elements in total.
Copying a `Record` costs two allocations (its string and its vector),
so that's over 2000 extra allocations that `noexcept` removes. For
types with big buffers, it's the difference between O(n) and O(n²)
bytes copied overall.

**`noexcept` is checked when it matters.** If a `noexcept` function
does throw, the program calls `std::terminate` immediately. It's a
real promise, so only make it for operations that can't fail. Moves
that just swap pointers can't.

**Destructors are `noexcept` by default.** Throwing from a destructor
during stack unwinding (an exception already in flight) would mean two
exceptions at once, so C++ makes destructors implicitly `noexcept`.
Don't throw from them.

**Checking at compile time.** `static_assert(std::is_nothrow_move_constructible_v<Record>);`
right below a class turns a forgotten `noexcept` into a build error.
Standard library implementations put checks like this all over their
own code.

**Other containers care too.** `std::deque` doesn't relocate elements
when it grows, so it doesn't need `noexcept` moves. `std::vector::reserve`,
`resize` and `insert` all go through the same move-or-copy decision.
