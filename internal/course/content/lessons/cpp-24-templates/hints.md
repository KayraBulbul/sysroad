`Stack` can keep its elements in a `std::vector<T> items_;`. The top is
`items_.back()`. `pop` copies (or moves) the back out, calls
`items_.pop_back()`, and returns it.
---
`count_where` takes two template parameters, the iterator type and the
predicate type:

```cpp
template <typename It, typename Pred>
std::size_t count_where(It first, It last, Pred pred) {
    std::size_t n = 0;
    for (; first != last; ++first) if (pred(*first)) ++n;
    return n;
}
```
It works for vector iterators, plain pointers, and list iterators alike.
