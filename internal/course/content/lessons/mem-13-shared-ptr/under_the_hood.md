**The control block.** A `shared_ptr` is two pointers: one to the
object and one to a *control block* holding the strong count, the weak
count and the deleter. `std::make_shared` allocates the object and its
control block in one go. `std::shared_ptr<T>(new T)` takes two
allocations. Prefer `make_shared`.

**The count is atomic.** Copies can happen on different threads, so the
count is updated with atomic instructions. That's noticeably slower than
a plain increment. Pass `shared_ptr` by `const&` when the function
doesn't keep a copy, and don't use it where `unique_ptr` would do.

**Why weak_ptr can tell the object is gone.** When the strong count hits
zero, the object is destroyed, but the control block stays while any
`weak_ptr` still points at it. That's how `lock()` can safely answer
"gone". The control block is freed when the weak count hits zero too.

**Leaks the sanitizer can't name.** A cycle of `shared_ptr`s is a leak,
and LeakSanitizer reports it on Linux. Without sanitizers, nothing ever
complains. Graph-shaped data (doubly linked lists, observers, parent
pointers) is where these hide.

**`enable_shared_from_this`.** An object that needs a `shared_ptr` to
*itself*, say to register itself with something, inherits from
`std::enable_shared_from_this<T>` and calls `shared_from_this()`. Never
do `std::shared_ptr<T>(this)`: that creates a second control block, and
the object gets deleted twice.
