**Copy-and-swap.** A popular way to write assignment is to take the
argument *by value*, which makes the copy with the copy constructor,
then swap its guts with yours:
`Buffer& operator=(Buffer other) { std::swap(data_, other.data_); std::swap(size_, other.size_); return *this; }`.
The old bytes leave in `other` and are freed by its destructor. It's
short, self-assignment safe and exception safe, though it always
allocates, even when you could have reused your buffer.

**When the compiler's copy is right.** If every member copies itself
correctly (`std::string`, `std::vector`, ints), the generated copy is
exactly what you want, and writing your own is just a chance to forget
a member. Only classes that manage a resource directly, like a raw
pointer, a file descriptor or a lock, need custom copying. Lesson 2.3.4
turns that into a rule.

**`= delete`.** Some things shouldn't be copied at all, like your `Fd`
or a mutex. `Buffer(const Buffer&) = delete;` makes copying a compile
error, which is far better than a double free at run time.

**Copies you didn't write.** Passing by value, returning a local,
`push_back(x)` and range-for without `&` all copy. `for (auto s : names)`
copies every string, and `for (const auto& s : names)` copies none.
That's the next lesson's topic.
