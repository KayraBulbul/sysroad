---
title: References vs pointers
minutes: 15
files: [refs.cpp]
deeper:
  - title: Reference declaration (cppreference)
    url: https://en.cppreference.com/w/cpp/language/reference
  - title: "C++ Core Guidelines: F.60 Prefer T* over T& when “no argument” is a valid option"
    url: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-ptr-ref
---
You know pointers from C. C++ adds **references**: another name for an
object that already exists.

```cpp
int x = 1;
int& r = x;   // r *is* x
r = 5;        // x is now 5
int* p = &x;  // a pointer holds x's address
*p = 7;       // x is now 7
```

Underneath, a reference is usually a pointer, but the language gives it
three rules that make it safer:

- It must be bound to something when it's made. There's no null reference.
- It can't be re-pointed. `r = y;` copies `y` into `x`.
- You use it like the object itself: no `*`, no `->`.

**Which one to use.** The everyday rule:

- Read a big object without copying: `const T&`
- Change the caller's object: `T&`
- Say "maybe there's nothing": `T*`, which can be `nullptr`
- Store a link you'll re-point later: `T*`

So a function that *finds* something returns a pointer, because finding
nothing is a normal answer. A function that *edits* its argument takes a
reference.

**Don't return references to locals.** The object dies when the function
returns. Returning a reference or pointer *into a parameter* is fine, as
long as the caller's object outlives the result.

## Assignment

- [ ] `swap_values` swaps two ints through references
- [ ] `find_value` returns a pointer to the first match, or `nullptr`
- [ ] `trim` removes leading and trailing whitespace from a string in place
- [ ] `longest` returns a pointer to the longest word (the first on a tie), or `nullptr` for an empty list
