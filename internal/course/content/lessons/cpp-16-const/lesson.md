---
title: const everywhere
minutes: 15
files: [inventory.hpp, inventory.cpp]
deeper:
  - title: cv type qualifiers (cppreference)
    url: https://en.cppreference.com/w/cpp/language/cv
  - title: constexpr specifier (cppreference)
    url: https://en.cppreference.com/w/cpp/language/constexpr
---
`const` is a promise the compiler holds you to, and it shows up in four
places:

```cpp
const int limit = 10;                  // a value that never changes
void show(const Inventory& inv);       // "I'll look, not touch"
int count(const std::string&) const;   // this member function only reads
constexpr std::size_t kib(std::size_t n) { return n * 1024; }  // works at compile time
```

The one that bites is the **const member function**. Once you have a
`const Inventory&`, which is what every "just looking" function takes,
you can only call members marked `const`:

```text
error: 'this' argument to member function 'size' has type
'const Inventory', but function is not marked const
```

`const` also spreads. Inside a `const` member, the fields are const too, so
`items_[name]` doesn't compile: `std::map::operator[]` *inserts* a missing
key, which changes the map. Use `find`, which only looks.

**This lesson's code works, but it doesn't compile against the tests.**
Run it and read the errors. They point at each missing `const`. Then fix
the bug the tests find: asking for an item that isn't there mustn't add
it.

`constexpr` means "can be worked out at compile time": the tests use
`kib` inside a `static_assert`, which is checked while compiling. A
`constexpr` function is still an ordinary function at run time.

## Assignment

- [ ] Mark every member that only reads as `const` (both in the header and the `.cpp`)
- [ ] Make `count` use `find` so it never inserts
- [ ] Make `kib` `constexpr`
- [ ] Make `total` take the inventory by `const&` instead of copying it
