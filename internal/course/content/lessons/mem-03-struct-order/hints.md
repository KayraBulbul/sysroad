`std::stable_sort` keeps equal elements in their original order. Give
it a lambda that says when `a` belongs before `b`:

```cpp
std::stable_sort(fields.begin(), fields.end(),
                 [](const Field& a, const Field& b) { return a.align > b.align; });
```
---
For `Record`, write down each field's alignment: `double` and
`std::int64_t` are 8, `float` is 4, `std::int16_t` is 2, `bool` and
`char` are 1. Then order them 8, 8, 4, 2, 1, 1. The program shows the same thing:
`./pack double int64 float int16 bool char` gives `sizeof 24`.
