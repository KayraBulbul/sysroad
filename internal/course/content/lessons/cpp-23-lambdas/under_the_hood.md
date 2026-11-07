**A lambda is a struct with an `operator()`.** The compiler turns
`[min](const Entry& e) { return e.size >= min; }` into a small unnamed
class with a `min` field and a call operator. Each lambda has its own
type, which is why you store them in `auto`. Because the type is known,
`std::sort` can inline the comparison completely. That's why
`std::sort` beats C's `qsort`, which calls through a function pointer every
time.

**std::sort is introsort.** It's quicksort that switches to heapsort if
the recursion gets too deep (so the worst case is still O(n log n)), and to
insertion sort for small ranges. It isn't stable: equal elements can
change order. That's why the tie-break on name is there. `std::stable_sort`
keeps equal elements in their original order.

**Capturing by reference can dangle.** A lambda that captures `[&]` and
is stored, returned, or run on another thread later holds references to
locals that may be gone by then. The rule: capture by reference only for
lambdas used immediately, like the ones passed to algorithms here.

**C++20 ranges** let you write `std::ranges::sort(v, cmp)` without
`begin()`/`end()`, and sort by a field with a *projection*:
`std::ranges::sort(v, std::greater{}, &Entry::size)`. Same algorithms,
less typing.
