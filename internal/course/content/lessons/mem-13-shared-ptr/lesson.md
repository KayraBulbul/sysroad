---
title: "std::shared_ptr and weak_ptr"
minutes: 25
files: [cache.cpp, family.hpp, family.cpp]
run: {}
deeper:
  - title: std::shared_ptr (cppreference)
    url: https://en.cppreference.com/w/cpp/memory/shared_ptr
  - title: std::weak_ptr (cppreference)
    url: https://en.cppreference.com/w/cpp/memory/weak_ptr
---
Sometimes there really is more than one owner. `std::shared_ptr<T>`
keeps a **reference count** next to the object: copying the pointer
adds one, and destroying a copy subtracts one. The last one out
deletes the object.

```cpp
auto a = std::make_shared<std::string>("hi");  // count 1
auto b = a;                                    // count 2: same string
a.reset();                                     // count 1
// b goes out of scope: count 0, the string is deleted
```

`std::weak_ptr<T>` **watches** a shared object without keeping it
alive. It doesn't add to the count. To use the object, call `lock()`,
which gives you a `shared_ptr` if the object still exists and an empty
one if it's gone.

That covers two jobs:

- **Caches.** A cache that held `shared_ptr`s would keep everything
  alive forever. One that holds `weak_ptr`s hands out the same object
  to everyone who asks while it's in use, and lets it go when nobody
  wants it.
- **Back-pointers.** A parent owns its children. If each child also
  held a `shared_ptr` to its parent, they'd keep each other's counts
  above zero forever: a **cycle**, and a leak that no destructor ever
  runs to fix. The child's link to its parent should be a `weak_ptr`.

`family.hpp` starts with exactly that cycle.

Reach for `unique_ptr` first. Use `shared_ptr` only when the lifetime
really is shared and you can't say who should free it.

## Assignment

- [ ] `Cache::get` returns the cached object while it's in use, and loads it again after it's gone
- [ ] `Cache::in_use` forgets expired entries and counts the rest
- [ ] Break the cycle in `family.hpp`: a child mustn't own its parent
- [ ] `adopt` links parent and child; `lineage` prints a name and its ancestors
