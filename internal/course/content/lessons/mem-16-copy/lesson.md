---
title: Copy constructors and copy assignment
minutes: 20
files: [buffer.cpp]
run: {}
deeper:
  - title: Copy constructors (cppreference)
    url: https://en.cppreference.com/w/cpp/language/copy_constructor
  - title: Copy assignment (cppreference)
    url: https://en.cppreference.com/w/cpp/language/copy_assignment
---
When you copy an object, C++ calls its **copy constructor**
(`Buffer b = a;`) or its **copy assignment operator** (`b = a;` when
`b` already exists). If you don't write them, the compiler writes them
for you, copying each member in turn.

For a class that owns heap memory through a raw pointer, that's wrong.
Copying `char* data_` copies the *address*, so both objects point at
the same bytes. Change one and the other changes. Destroy both and the
bytes are deleted twice. What you want is a **deep copy**: new memory,
with the bytes copied into it.

```cpp
Buffer::Buffer(const Buffer& other)
    : data_(new char[other.size_]), size_(other.size_) {
    std::memcpy(data_, other.data_, size_);
}
```

Assignment has two extra jobs. The object already owns memory, which
must be freed. And `a = a;` must still work, so don't free your own
bytes before copying from them. The safe order is to allocate and copy
first, then delete the old buffer, then take over the new one. If the
`new` throws, the object is still untouched.

`buffer.cpp` starts with what the compiler would have generated, plus
an empty destructor. Run it to see the shallow copy problem, then fix
it.

## Assignment

- [ ] The destructor frees the bytes
- [ ] The copy constructor makes a deep copy
- [ ] Copy assignment frees the old bytes, copies the new ones and survives `a = a`
