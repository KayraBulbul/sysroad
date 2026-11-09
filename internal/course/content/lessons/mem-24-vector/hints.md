`reserve`, the heart of it:

```cpp
void reserve(std::size_t n) {
    if (n <= cap_) return;
    T* fresh = static_cast<T*>(::operator new(n * sizeof(T)));
    for (std::size_t i = 0; i < size_; ++i) {
        new (fresh + i) T(std::move_if_noexcept(data_[i]));
        data_[i].~T();
    }
    ::operator delete(data_);
    data_ = fresh;
    cap_ = n;
}
```
---
`emplace_back` when there's room is one placement new:
`T* p = new (data_ + size_) T(std::forward<Args>(args)...); ++size_; return *p;`.
When it's full, grow to `cap_ == 0 ? 1 : 2 * cap_`, but construct the
new element in the new buffer *before* moving the old ones, so `args`
can still refer to them. Then both `push_back`s become
`emplace_back(value)` and `emplace_back(std::move(value))`.
---
`clear` destroys every element and keeps the buffer:
`for (std::size_t i = 0; i < size_; ++i) data_[i].~T(); size_ = 0;`.
The destructor is `clear()` plus `::operator delete(data_)`.
`resize(n)` either destroys from the end or `emplace_back()`s
value-initialised elements until the size is `n`.
---
The copy constructor reserves `other.size_`, then placement-news a copy
of each element. Copy assignment can be copy-and-swap:
`Vector tmp(other); swap(tmp); return *this;`, with a `swap` member
that `std::swap`s the three fields. The moves just steal the three
fields and zero `other`'s.
