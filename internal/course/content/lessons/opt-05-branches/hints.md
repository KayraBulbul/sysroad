`filter_above`:

```cpp
std::size_t count = 0;
for (std::size_t i = 0; i < n; ++i) {
    out[count] = in[i];
    count += in[i] > limit;
}
return count;
```

The last write may be one that wasn't kept. That's fine: it's inside
`out`, which has room for `n`, and `count` says where the real
answer ends.
---
`lower_bound_index`: after the loop, `n` is 1 (or 0, if the array was
empty), and the answer is either `base` or the element just after it.
`base - a` is `base`'s index:

```cpp
return static_cast<std::size_t>(base - a) + (n == 1 && *base < x);
```

The condition in the loop goes inside the builtin:
`base = __builtin_unpredictable(base[half - 1] < x) ? base + half : base;`
