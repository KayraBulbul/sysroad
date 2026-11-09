---
title: "lvalues, rvalues and std::move"
minutes: 20
files: [moves.cpp]
run: {}
deeper:
  - title: Value categories (cppreference)
    url: https://en.cppreference.com/w/cpp/language/value_category
  - title: std::move (cppreference)
    url: https://en.cppreference.com/w/cpp/utility/move
---
Copying a `std::string` means allocating and copying its characters.
But when the original is about to be destroyed anyway, that's wasted
work. You could just take its buffer. That's a **move**: the new object
steals the old one's insides and leaves it empty but valid.

C++ decides whether it may steal based on the expression's **value
category**:

- an **lvalue** has a name and lives on: `s`, `v[0]`, `*p`. Stealing
  from it would surprise whoever uses it next, so it gets copied.
- an **rvalue** is a temporary that's about to die: `std::string("hi")`,
  `make_name()`, `a + b`. Nobody can see it again, so it gets moved.

`T&&` is an *rvalue reference*: it only binds to rvalues. That's how a
move constructor, `Tracked(Tracked&& other)`, gets picked for
temporaries while the copy constructor handles everything else.

`std::move(x)` doesn't move anything. It's a cast that says "treat `x`
as an rvalue, I'm done with it". The move happens when that rvalue
lands in a constructor or assignment. Afterwards `x` is still a valid
object in an *unspecified* state. You can assign to it or destroy it,
but don't read it.

```cpp
std::string name = "a long name that lives on the heap";
names.push_back(name);             // copy: name is still needed
names.push_back(std::move(name));  // move: name is now empty-ish
```

`tracked.hpp` defines `Tracked`, a string wrapper that counts its own
copies and moves. Every function in `moves.cpp` works, but copies
where it doesn't have to. The tests want zero copies.

## Assignment

- [ ] `append` moves its parameter into the vector
- [ ] `take_last` moves the last element out before popping it
- [ ] `concat` moves `b`'s elements onto the end of `a`
- [ ] `rotate3` uses only moves
- [ ] `longest` stops copying in its loop
