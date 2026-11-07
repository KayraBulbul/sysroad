---
title: Your first template
minutes: 20
files: [generic.hpp, main.cpp]
run: {}
deeper:
  - title: Templates (cppreference)
    url: https://en.cppreference.com/w/cpp/language/templates
  - title: Constraints and concepts (cppreference)
    url: https://en.cppreference.com/w/cpp/language/constraints
---
`std::vector<int>` and `std::vector<std::string>` are the same code,
written once. That's a **template**: code with a type left blank, which the
compiler fills in for each type you use.

```cpp
template <typename T>
T bigger(const T& a, const T& b) {
    return a < b ? b : a;
}

bigger(3, 7);                                    // T = int
bigger(std::string("ab"), std::string("b"));     // T = std::string
```

The compiler works out `T` from the arguments, then generates
(*instantiates*) a separate `bigger<int>` and `bigger<std::string>`.

**Templates live in headers.** The compiler needs the whole body every
time it instantiates one, so it can't be tucked away in a `.cpp`. That's
why this lesson's code goes in `generic.hpp`.

**Concepts say what `T` must support.** Without one, a bad type produces
pages of errors from deep inside your template. With one, you get a single
line at the call:

```cpp
template <std::totally_ordered T>   // T must support < > <= >= == !=
std::optional<T> max_of(const std::vector<T>& v);
```

`std::totally_ordered` is from `<concepts>`. Try `max_of` on a vector of a
struct with no `<` and read the error.

A class template looks the same: `template <typename T> class Stack { ...
};`, with `T` used inside like any type.

## Assignment

- [ ] `max_of`: the largest element (the first on a tie), or `nullopt` for an empty vector
- [ ] `Stack<T>`: `push`, `pop` (returns the top, or `nullopt` when empty), `top`, `size`, `empty`
- [ ] `count_where`: how many elements in `[first, last)` make `pred` return true, for any iterator type
