The destructor is one line: `delete[] data_;`. Deleting `nullptr` is
fine, so an empty `Buffer` needs no check.
---
Once the destructor frees, the starter's shallow copy constructor turns
into a double free: two `Buffer`s, one pointer, two `delete[]`s. The
lesson shows the deep copy constructor. `std::memcpy` with a size of 0
is fine.
---
Assignment, in the safe order:

```cpp
Buffer& Buffer::operator=(const Buffer& other) {
    if (this == &other) return *this;
    char* fresh = new char[other.size_];
    std::memcpy(fresh, other.data_, other.size_);
    delete[] data_;
    data_ = fresh;
    size_ = other.size_;
    return *this;
}
```

The self-check isn't needed for correctness with this order, but it
saves a pointless allocation.
