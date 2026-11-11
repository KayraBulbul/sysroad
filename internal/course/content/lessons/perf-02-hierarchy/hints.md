The constructor makes `sets()` empty vectors. A vector's size
constructor does that, in the initializer list:

```cpp
Cache::Cache(CacheConfig config) : config_(config), sets_(sets()) {}
```

`sets()` only reads `config_`, which is already set by then, because
members are initialised in the order they're declared.
---
`access` bumps `time_`, then looks through `sets_[set_of(addr)]` for a
line with `tag_of(addr)`. If it's there, set its `last_used` to `time_`,
count a hit and return true.
---
On a miss, count it. If the set has fewer than `config_.ways` lines,
`push_back` the new one. Otherwise find the line with the smallest
`last_used` and overwrite it with the new tag and `time_`.
