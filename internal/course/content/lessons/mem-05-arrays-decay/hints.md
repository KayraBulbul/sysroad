`sum` is the loop from the lesson. `p` walks from `first` to `last`,
and `*p` is the int it points at.
---
Two pointers that walk towards each other. `last` is one past the end,
so step it back before you use it:

```cpp
while (first != last && first != --last) {
    std::swap(*first, *last);
    ++first;
}
```

The `first != --last` check stops when they meet in the middle.
---
A `std::array` parameter taken by value is already a copy, so change it
and return it: `for (int& x : a) x *= 2; return a;`.
