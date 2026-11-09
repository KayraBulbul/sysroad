Put the growth in one place. A private helper is handy, but `reserve`
can be it:

```cpp
void String::reserve(std::size_t n) {
    if (n <= cap_) return;
    char* fresh = new char[n + 1];
    std::memcpy(fresh, data(), size_ + 1);   // + 1: the '\0'
    delete[] heap_;                          // nullptr while inline: fine
    heap_ = fresh;
    cap_ = n;
}
```
---
`push_back` and `append` grow to at least double:
`if (size_ + extra > cap_) reserve(std::max(size_ + extra, 2 * cap_));`.
Then copy the new chars to `data() + size_`, add to `size_`, and write
the `'\0'` at `data()[size_]`.
---
For `append`, aliasing breaks this if `s` points into your own buffer:
`reserve` frees it before you copy. Either check
`if (s.data() >= data() && s.data() <= data() + size_)` and copy `s` into
a temporary `std::string` first, or write `append`'s growth by hand:
allocate, copy your old text, copy `s`, then free the old buffer.
---
Moves. If `other` is on the heap, take its pointer and capacity, and
reset it to empty inline (`heap_ = nullptr; size_ = 0; cap_ = 15; buf_[0] = '\0';`).
If it's inline, copy its `buf_`, since there's no pointer to steal. In
move *assignment*, free your own heap buffer first, and do nothing for
`s = std::move(s)`.
---
Copy assignment that reuses your buffer when it's big enough:

```cpp
if (this == &other) return *this;
if (other.size_ > cap_) {     // too small: replace the buffer
    char* fresh = new char[other.size_ + 1];
    delete[] heap_;
    heap_ = fresh;
    cap_ = other.size_;
}
std::memcpy(data(), other.data(), other.size_ + 1);
size_ = other.size_;
```
