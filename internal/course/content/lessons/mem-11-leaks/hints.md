`sum_numbers` returns early from inside the loop, past the `delete[]`.
Either free before that `return -1`, or (better) don't use `new` at all:
a `std::vector<int>` frees itself on every path.
---
In `shout`, the `delete[]` comes before the last use of `line`. Build
the `std::string` first, then free. Or skip the raw buffer: copy the
first line into a `std::string` and upper-case that in place.
---
`doubled_copy` promises `nullptr` for `n == 0`, but it frees `out` and
then returns it anyway. The caller then frees it a second time.
Return `nullptr` there (or never allocate when `n` is 0).
---
`new std::string[cap]` needs `delete[]`. With `delete`, only one
destructor runs and the pointer handed back isn't the one the allocator
gave out: arrays of objects with destructors store their length just
before the first element. clang warns about this one:
`'delete' applied to a pointer that was allocated with 'new[]'`.
